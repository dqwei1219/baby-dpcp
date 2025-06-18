#pragma once

#include "Connection.h"
#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <memory>
#include <functional>
#include <condition_variable>
#include <thread>

class ConnectionPool
{
    public:
        static ConnectionPool& getConnectionPool() {
            static ConnectionPool instance;
            return instance;
        }

        // Delete Copy and Move
        ConnectionPool(const ConnectionPool&) = delete;
        ConnectionPool& operator=(const ConnectionPool&) = delete;
        ConnectionPool(ConnectionPool&&) = delete;
        ConnectionPool& operator=(ConnectionPool&&) = delete;

        // get an available connection;
        std::shared_ptr<Connection> getConnection();
        struct Stats {
            size_t totalConnections;
            size_t availableConnections;
            size_t activeConnections;
            uint64_t totalRequests;
            uint64_t timeoutCount;
        };
        Stats getStats() const;

    private:
        ConnectionPool(); // Singleton
        ~ConnectionPool();
        bool loadConfig(const std::string& filename);
        void producerThread();
        void sweeperThread(); // Restore connection when exceed max idle time
        void initialize();
        void shutdown();
        std::unique_ptr<Connection> createConnection();

        // Configuration
        struct Config {
            std::string host{"localhost"};
            uint16_t port{3306};
            std::string database;
            std::string username;
            std::string password;
            int minSize{5};
            int maxSize{20};
            std::chrono::seconds maxIdleTime{60};
            std::chrono::milliseconds connectionTimeout{5000};
        };

        Config _config;
        mutable std::mutex _mu;
        std::thread _producer;
        std::thread _sweeper;
        std::queue<std::unique_ptr<Connection>> _availableConnections;
        std::atomic<int> _activeConnections{0};
        std::atomic<bool> _shutdown{false};
        std::condition_variable _notEmpty;
        std::condition_variable _notFull;
        
        // Statistics
        std::atomic<int> _totalRequests{0};
        std::atomic<int> _timeoutCount{0};

};