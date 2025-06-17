#include <iostream>
#include <chrono>
#include "CommonConnectionPool.h"

int main() {
    std::cout << "Testing WITH Connection Pool\n";
    std::cout << "===========================\n\n";
    
    try {
        ConnectionPool& pool = ConnectionPool::getConnectionPool();
        
        auto start = std::chrono::high_resolution_clock::now();
        
        int successCount = 0;
        for (int i = 0; i < 1000; ++i) {
            auto conn = pool.getConnection();
            if (!conn) {
                std::cout << "Failed to get connection at iteration " << i << std::endl;
                continue;
            }
            
            std::string sqlQuery = "INSERT INTO user(name, age, sex) VALUES('wang', 25, 'female')";
            if (conn->update(sqlQuery)) {
                successCount++;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double, std::milli>(end - start);
        
        std::cout << "Successful operations: " << successCount << "/1000\n";
        std::cout << "Total time: " << duration.count() << " ms\n";
        std::cout << "Average per operation: " << duration.count() / 1000 << " ms\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}