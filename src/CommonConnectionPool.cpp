#include "CommonConnectionPool.h"
#include "public.h"
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <thread>
#include <condition_variable>

ConnectionPool* ConnectionPool::getConnectionPool()
{
    static ConnectionPool pool; // lock and unlock
    return &pool;
}

bool ConnectionPool::loadConfig()
{
    std::ifstream f("mysql.config");
    if (!f.is_open()) {
        LOG("File mysql.config doesn't exist");
        return false;
    }

    std::string line;
    while(std::getline(f, line)) {
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        size_t pos = line.find("=");
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value  = line.substr(pos+1, line.length());
            _config[key] = value;
        }
    }
    f.close();

    _port = static_cast<uint16_t>(std::stoi(get("port", "3306")));
    _dbname = get("dbname");
    _username = get("username");
    _password = get("password");
    _initSize = std::stoi(get("initSize", "10"));
    _maxSize = std::stoi(get("maxSize", "1024"));
    _maxIdleTime = std::stoi(get("maxIdleTime", "60"));
    _connectionTimeout = std::stoi(get("connectionTimeout", "100"));

    if (_username.empty() || _password.empty() || _dbname.empty() ) {
        LOG("No username and password loading config");
        return false;
    }
    
    return true;
}

void ConnectionPool::produceConnection() {
    for(;;) {
        std::unique_lock<std::mutex> lock(_qMutex);
        while(!_connectionQ.empty()) {
            cv.wait(lock);
        }

        if (_connectionCnt < _maxSize) {
            Connection* p = new Connection();
            p->connect(_ip, _port, _username, _password, _dbname);
            _connectionQ.push(p);
            _connectionCnt++;
        }

        cv.notify_all();
    }

}

ConnectionPool::ConnectionPool() {
    if (!loadConfig()) {
        return;
    }

    // initial size;
    for (int i = 0; i < _initSize; ++i) {
        Connection *p = new Connection();
        p->connect(_ip, _port, _username, _password, _dbname);
        _connectionQ.push(p);
        _connectionCnt++;
    }

    // create thread produce connections when needed
    std::thread produce(std::bind(&ConnectionPool::produceConnection, this));
}

std::shared_ptr<Connection> ConnectionPool::getConnection()
{
    std::unique_lock<std::mutex> lock(_qMutex);
    if (_connectionQ.empty()) {
        cv.wait_for(lock, std::chrono::milliseconds(_connectionTimeout));
        if (_connectionQ.empty()) {
            LOG("Connection acquire timeout");
            return nullptr;
        }
    }

    // Customize shared pointer by not deleting the connection but instead return the pool
    std::shared_ptr<Connection> sp(_connectionQ.front(), [&](Connection *pcon) {
        std::unique_lock<std::mutex> lock(_qMutex);
        _connectionQ.push(pcon);
    }); 

    _connectionQ.pop();
    // notify producer if more connection is required.
    if (_connectionQ.empty()) {
        cv.notify_all();
    }

    return sp;
}
