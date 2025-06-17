#pragma once 
#include <string>
#include <chrono>
#include <cppconn/driver.h>
#include <cppconn/resultset.h>

namespace sql {
    class Statement;
    class PreparedStatement;
}

class Connection {
    public:
        Connection();
        ~Connection();
        bool connect(std::string ip,
            unsigned short port,
            std::string user,
            std::string password,
            std::string dbname);

        // insert, delete, update
        bool update(std::string sql);
        // select
        std::unique_ptr<sql::ResultSet> query(std::string sql);

        bool isConnected() const;
        void disconnect();
        void refreshAliveTime() { _aliveTime = std::chrono::high_resolution_clock::now(); }
        std::chrono::seconds getAliveTime() const {
            auto now = std::chrono::high_resolution_clock::now();
            return std::chrono::duration_cast<std::chrono::seconds>(now - _aliveTime);
        }

    private:
        std::unique_ptr<sql::Connection> _conn;
        sql::Driver* _driver;
        std::chrono::time_point<std::chrono::high_resolution_clock> _aliveTime;
};