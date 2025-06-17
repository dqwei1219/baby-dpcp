#include <iostream>
#include <chrono>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include "Connection.h"

int main() {
    std::string host = "localhost";
    unsigned short port = 3306;
    std::string user = "testuser";
    std::string password = "Test@1234";
    std::string database = "testdb";

    auto begin = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i) {
        Connection db_conn;
        if(!db_conn.connect(host, port, user, password, database)) {
            std::cout << "Connection failed" << std::endl;
            return 1;
        }
        // Create connection
        std::string sqlQuery = "INSERT INTO user(name, age, sex) VALUES('zhang', 20, 'male')";
        db_conn.update(sqlQuery);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto totalDuration = std::chrono::duration<double, std::milli>(end - begin);
    std::cout << totalDuration.count() << "ms" << std::endl;

    return 0;
}