#pragma once

#include "Connection.h"
#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <memory>
#include <functional>

class ConnectionPool
{
    public:
        static ConnectionPool* getConnectionPool();
        // get an available connection;
        std::shared_ptr<Connection> getConnection();

    private:
        ConnectionPool(); // Singleton
        bool loadConfig();
        void produceConnection();


        std::string _ip;
        unsigned short _port;
        std::string _dbname;
        std::string _username;
        std::string _password;

        int _initSize;
        int _maxSize;
        int _maxIdleTime;
        int _connectionTimeout;
        std::queue<Connection*> _connectionQ;
        std::mutex _qMutex;
        std::unordered_map<std::string, std::string> _config;
        std::atomic_int _connectionCnt;
        std::condition_variable cv;

        std::string get(std::string key, std::string defaultValue="") const {
            auto it = _config.find(key);
            if (it != _config.end()) {
                return it->second;
            }
            return defaultValue;
        }
};