#include "DatabaseServer.h"
#include <spdlog/spdlog.h>
#include <chrono>

using json = nlohmann::json;

DatabaseServer::DatabaseServer(const std::string& auth_token)
    : pool_(ConnectionPool::getConnectionPool()), auth_token_(auth_token) {
    setupRoutes();
}

void DatabaseServer::setupRoutes() {
    server_.set_pre_routing_handler([this](const httplib::Request& req, httplib::Response& res) {
        if (req.path == "/health") return httplib::Server::HandlerResponse::Unhandled;

        if (!authenticate(req)) {
            res.status = 401;
            res.set_content("{\"error\":\"Unauthorized\"}", "application/json");
            return httplib::Server::HandlerResponse::Handled;
        }
        return httplib::Server::HandlerResponse::Unhandled;
    });

    server_.Get("/health", [this](const httplib::Request&, httplib::Response& res) {
        json response;
        response["status"] = "healthy";
        response["pool_stats"] = getPoolStats();
        res.set_content(response.dump(), "application/json");
    });

    server_.Post("/query", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            json request = json::parse(req.body);
            std::string sql = request["sql"];
            auto params = request.value("params", json::array());

            auto conn = pool_.getConnection();
            if (!conn) throw std::runtime_error("No connection available");

            spdlog::debug("Executing query: {}", sql);

            auto start = std::chrono::high_resolution_clock::now();
            auto results = conn->query(sql);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            json response;
            response["data"] = convertResultSet(results);
            response["execution_time_ms"] = duration.count();
            res.set_content(response.dump(), "application/json");
        } catch (const std::exception& e) {
            handleError(res, e);
        }
    });

    server_.Post("/execute", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            json request = json::parse(req.body);
            std::string sql = request["sql"];

            auto conn = pool_.getConnection();
            if (!conn) throw std::runtime_error("No connection available");

            bool success = conn->update(sql);

            json response;
            response["success"] = success;
            res.set_content(response.dump(), "application/json");
        } catch (const std::exception& e) {
            handleError(res, e);
        }
    });
}

bool DatabaseServer::authenticate(const httplib::Request& req) {
    auto auth_header = req.get_header_value("Authorization");
    return auth_header == "Bearer " + auth_token_;
}

json DatabaseServer::getPoolStats() {
    auto stats = pool_.getStats();
    json result;
    result["total_connections"] = stats.totalConnections;
    result["available_connections"] = stats.availableConnections;
    result["active_connections"] = stats.activeConnections;
    result["total_requests"] = stats.totalRequests;
    result["timeout_count"] = stats.timeoutCount;
    return result;
}

json DatabaseServer::convertResultSet(std::unique_ptr<sql::ResultSet>& rs) {
    json rows = json::array();
    if (!rs) return rows;

    auto metadata = rs->getMetaData();
    int columnCount = metadata->getColumnCount();
    std::vector<std::string> columns;
    for (int i = 1; i <= columnCount; ++i) {
        columns.push_back(metadata->getColumnLabel(i));
    }

    while (rs->next()) {
        json row;
        for (int i = 1; i <= columnCount; ++i) {
            switch (metadata->getColumnType(i)) {
                case sql::DataType::INTEGER:
                    row[columns[i - 1]] = rs->getInt(i);
                    break;
                case sql::DataType::DOUBLE:
                    row[columns[i - 1]] = rs->getDouble(i);
                    break;
                case sql::DataType::BINARY:
                    row[columns[i - 1]] = rs->getBoolean(i);
                    break;
                default:
                    row[columns[i - 1]] = rs->getString(i);
            }
        }
        rows.push_back(row);
    }

    return rows;
}

void DatabaseServer::handleError(httplib::Response& res, const std::exception& e) {
    spdlog::error("Request error: {}", e.what());
    res.status = 500;
    json error;
    error["error"] = e.what();
    res.set_content(error.dump(), "application/json");
}

void DatabaseServer::start(int port) {
    spdlog::info("Starting database server on port {}", port);
    server_.listen("0.0.0.0", port);
}

void DatabaseServer::stop() {
    server_.stop();
}
