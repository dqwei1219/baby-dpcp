#pragma once 
#include <string>
#include <cppconn/driver.h>
#include <cppconn/resultset.h>

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

    private:
        std::unique_ptr<sql::Connection> _conn;
        sql::Driver* _driver;
};