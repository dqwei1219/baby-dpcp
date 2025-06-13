#include <iostream>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include "Connection.h"

int main() {
    Connection db_conn;
    try {
        std::cout << "Getting driver instance..." << std::endl;
        std::cout << "Driver instance obtained!" << std::endl;
        
        std::cout << "Connecting to MySQL..." << std::endl;
        std::string host = "localhost";
        unsigned short port = 3306;
        std::string user = "testuser";
        std::string password = "Test@1234";
        std::string database = "testdb";
        if(!db_conn.connect(host, port, user, password, database)) {
            std::cout << "Connection failed" << std::endl;
            return 1;
        }
        std::cout << "Connected!" << std::endl;
        
    } catch (sql::SQLException &e) {
        std::cout << "ERROR: " << e.what() << std::endl;
        std::cout << "MySQL error code: " << e.getErrorCode() << std::endl;
    } catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    std::cout << "Done." << std::endl;
    return 0;
}