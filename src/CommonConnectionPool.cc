#include "CommonConnectionPool.h"
#include "public.h"
#include <fstream>
#include <sstream>
#include <algorithm>

ConnectionPool::ConnectionPool() {
    if (!loadConfig("tests/mysql.config")) {
        throw std::runtime_error("Failed to load connection pool config");
    }
    initialize();
}

ConnectionPool::~ConnectionPool() {
    shutdown();
}

bool ConnectionPool::loadConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        LOG("Failed to open config file: " + filename);
        return false;
    }

    std::unordered_map<std::string, std::string> configMap;
    std::string line;
    
    while (std::getline(file, line)) {
        std::cout << line << std::endl;
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            configMap[key] = value;
        }
    }
    std::cout << std::endl;

    // Parse configuration with defaults
    auto getConfig = [&](const std::string& key, const std::string& defaultValue = "") {
        auto it = configMap.find(key);
        return it != configMap.end() ? it->second : defaultValue;
    };

    _config.host = getConfig("host", "localhost");
    _config.port = static_cast<uint16_t>(std::stoi(getConfig("port", "3306")));
    _config.database = getConfig("dbname");
    _config.username = getConfig("user");
    _config.password = getConfig("password");
    _config.minSize = std::stoul(getConfig("initSize", "5"));
    _config.maxSize = std::stoul(getConfig("maxSize", "20"));
    _config.maxIdleTime = std::chrono::seconds(std::stoi(getConfig("maxIdleTime", "60")));
    _config.connectionTimeout = std::chrono::milliseconds(std::stoi(getConfig("connectionTimeout", "5000")));

    // Validate required fields
    if (_config.database.empty() || _config.username.empty() || _config.password.empty()) {
        LOG("Missing required database credentials");
        return false;
    }

    // Validate sizes
    if (_config.minSize > _config.maxSize) {
        LOG("Invalid pool size configuration");
        return false;
    }

    return true;
}

void ConnectionPool::initialize() {
    std::lock_guard<std::mutex> lock(_mu);
    
    // Create initial connections
    for (int i = 0; i < _config.minSize; ++i) {
        auto conn = createConnection();
        if (conn) {
            _availableConnections.push(std::move(conn));
        } else {
            LOG("Failed to create initial connection");
            throw std::runtime_error("Failed to initialize connection pool");
        }
    }

    // Start background threads
    _producer = std::thread(std::bind(&ConnectionPool::producerThread, this));
    _sweeper = std::thread(std::bind(&ConnectionPool::sweeperThread, this));

    LOG("Connection pool initialized with " + std::to_string(_config.minSize) + " connections");
}

void ConnectionPool::shutdown() {
    {
        std::lock_guard<std::mutex> lock(_mu);
        _shutdown = true;
    }
    
    _notEmpty.notify_all();
    _notFull.notify_all();

    if (_producer.joinable()) {
        _producer.join();
    }
    if (_sweeper.joinable()) {
        _sweeper.join();
    }

    // Clear all connections
    std::lock_guard<std::mutex> lock(_mu);
    while (!_availableConnections.empty()) {
        _availableConnections.pop();
    }
}

std::unique_ptr<Connection> ConnectionPool::createConnection() {
    auto conn = std::make_unique<Connection>();
    
    if (conn->connect(_config.host, _config.port, _config.username, 
                     _config.password, _config.database)) {
        conn->refreshAliveTime();
        return conn;
    }
    
    return nullptr;
}

std::shared_ptr<Connection> ConnectionPool::getConnection() {
    _totalRequests++;
    
    std::unique_lock<std::mutex> lock(_mu);
    
    // Wait for available connection
    auto deadline = std::chrono::steady_clock::now() + _config.connectionTimeout;
    
    while (_availableConnections.empty() && !_shutdown) {
        if (_notEmpty.wait_until(lock, deadline) == std::cv_status::timeout) {
            _timeoutCount++;
            LOG("Connection acquisition timeout");
            return nullptr;
        }
    }
    
    if (_shutdown || _availableConnections.empty()) {
        return nullptr;
    }
    
    // Get connection from queue
    auto conn = std::move(_availableConnections.front());
    _availableConnections.pop();
    
    // Validate connection
    if (!conn->isConnected()) {
        // Only create new one if the pooled one is dead
        conn = createConnection();
        if (!conn) {
            return nullptr;
        }
    }
    
    _activeConnections++;
    conn->refreshAliveTime();
    
    // Create shared_ptr with custom deleter
    auto* rawConn = conn.release();
    return std::shared_ptr<Connection>(rawConn, 
        [this](Connection* c) {
            std::lock_guard<std::mutex> lock(_mu);
            if (!_shutdown) {
                _availableConnections.push(std::unique_ptr<Connection>(c));
                _activeConnections--;
                _notEmpty.notify_one();
            } else {
                delete c;
            }
        });
}

void ConnectionPool::producerThread() {
    while (!_shutdown) {
        std::unique_lock<std::mutex> lock(_mu);
        
        // Wait if we have enough connections
        _notFull.wait(lock, [this] {
            return _shutdown|| 
                   (static_cast<int>(_availableConnections.size()) + _activeConnections <= _config.maxSize &&
                    static_cast<int>(_availableConnections.size()) < _config.minSize);
        });
        
        if (_shutdown) break;
        
        // Create new connection
        lock.unlock();
        auto conn = createConnection();
        lock.lock();
        
        if (conn && !_shutdown) {
            _availableConnections.push(std::move(conn));
            _notEmpty.notify_one();
        }
    }
}

void ConnectionPool::sweeperThread() {
    while (!_shutdown) {
        std::this_thread::sleep_for(std::chrono::seconds(_config.maxIdleTime));
        
        std::lock_guard<std::mutex> lock(_mu);
        
        if (_shutdown) break;
        
        // Don't sweep below minimum size
        while (static_cast<int>(_availableConnections.size()) > _config.minSize) {
            auto& conn = _availableConnections.front();
            
            if (conn->getAliveTime() > _config.maxIdleTime) {
                _availableConnections.pop();
                LOG("Swept idle connection");
            } else {
                // Queue is ordered, so we can stop
                break;
            }
        }
    }
}