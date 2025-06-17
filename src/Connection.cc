#include "Connection.h"
#include "public.h"
#include "cppconn/statement.h"
#include "cppconn/prepared_statement.h"
#include <iostream>
#include <sstream>

Connection::Connection() : _conn(nullptr), _driver(nullptr) {
    try {
        _driver = get_driver_instance();
    } catch (sql::SQLException& e) {
        LOG("Failed to get driver instance: " + std::string(e.what()));
    }
}

Connection::~Connection() {
    disconnect();
}

bool Connection::connect(std::string ip,
                        unsigned short port,
                        std::string user,
                        std::string password,
                        std::string dbname) {
    try {
        if (!_driver) {
            return false;
        }
        
        // Build connection string
        std::ostringstream connectionString;
        connectionString << "tcp://" << ip << ":" << port;
        
        // Create connection
        _conn.reset(_driver->connect(connectionString.str(), user, password));
        
        if (!_conn) {
            LOG("Failed to create connection");
            return false;
        }
        
        // Set database schema
        _conn->setSchema(dbname);

        return true;
        
    } catch (sql::SQLException& e) {
        LOG("Connection failed: " + std::string(e.what()) + 
                    " (Error code: " + std::to_string(e.getErrorCode()) + ")");
        return false;
    }
}

void Connection::disconnect() {
    if (_conn) {
        try {
            _conn->close();
        } catch (sql::SQLException& e) {
            std::cerr << "Error during disconnect: " << e.what() << std::endl;
        }
        _conn.reset();
    }
}

bool Connection::isConnected() const {
    return _conn && !_conn->isClosed();
}

bool Connection::update(std::string sql) {
    if (!isConnected()) {
        LOG("Not connected to database");
        return false;
    }
    
    try {
        std::unique_ptr<sql::Statement> stmt(_conn->createStatement());
        int affectedRows = stmt->executeUpdate(sql);
        
        return affectedRows >= 0;
        
    } catch (sql::SQLException& e) {
        LOG("Update failed: " + std::string(e.what()) + 
                    " (Error code: " + std::to_string(e.getErrorCode()) + ")");
        return false;
    }

    return true;
}

std::unique_ptr<sql::ResultSet> Connection::query(std::string sql) {
    if (!isConnected()) {
        LOG("Not connected to database");
        return nullptr;
    }

    try {
        std::unique_ptr<sql::Statement> stmt(_conn->createStatement());
        return std::unique_ptr<sql::ResultSet>(stmt->executeQuery(sql));
    } catch (sql::SQLException& e) {
        LOG("Update failed: " + std::string(e.what()) + 
                    " (Error code: " + std::to_string(e.getErrorCode()) + ")");
        return nullptr;
    }
}