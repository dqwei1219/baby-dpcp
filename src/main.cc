#include <iostream>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

int main() {
    try {
        std::cout << "Getting driver instance..." << std::endl;
        sql::Driver *driver = get_driver_instance();
        std::cout << "Driver instance obtained!" << std::endl;
        
        // Uncomment to test connection if you have MySQL server set up
        // std::cout << "Connecting to MySQL..." << std::endl;
        // std::unique_ptr<sql::Connection> con(driver->connect("tcp://127.0.0.1:3306", "username", "password"));
        // std::cout << "Connected!" << std::endl;
        
    } catch (sql::SQLException &e) {
        std::cout << "ERROR: " << e.what() << std::endl;
        std::cout << "MySQL error code: " << e.getErrorCode() << std::endl;
    } catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    std::cout << "Done." << std::endl;
    return 0;
}