#pragma once

#include <string>
#include <httplib.h>
#include "json.hpp"
#include "CommonConnectionPool.h"

class DatabaseServer {
public:
    explicit DatabaseServer(const std::string& auth_token);
    void start(int port);
    void stop();

private:
    httplib::Server server_;
    ConnectionPool& pool_;
    std::string auth_token_;

    void setupRoutes();
    bool authenticate(const httplib::Request& req);
    nlohmann::json getPoolStats();
    nlohmann::json convertResultSet(std::unique_ptr<sql::ResultSet>& rs);
    void handleError(httplib::Response& res, const std::exception& e);
};