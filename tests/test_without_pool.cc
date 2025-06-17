#include <iostream>
#include <chrono>
#include <atomic>
#include "Connection.h"
#include <vector>
#include <thread>

void testConcurrentWithoutPool() {
    std::cout << "\n=== Concurrent Test WITHOUT Pool ===" << std::endl;
    
    const int numThreads = 10;
    const int operationsPerThread = 100;
    std::atomic<int> failureCount{0};
    
    auto threadFunc = [&failureCount]() {
        std::string host = "localhost";
        unsigned short port = 3306;
        std::string user = "testuser";
        std::string password = "Test@1234";
        std::string database = "testdb";
        
        for (int i = 0; i < operationsPerThread; ++i) {
            Connection db_conn;
            if (!db_conn.connect(host, port, user, password, database)) {
                failureCount++;
                continue;
            }
            
            std::string sqlQuery = "INSERT INTO user(name, age, sex) VALUES('li', 30, 'male')";
            db_conn.update(sqlQuery);
        }
    };
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(threadFunc);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start);
    
    std::cout << "Time taken: " << duration.count() << " ms" << std::endl;
    std::cout << "Total operations: " << numThreads * operationsPerThread << std::endl;
    std::cout << "Average per operation: " << duration.count() / (numThreads * operationsPerThread) << " ms" << std::endl;
    std::cout << "Failures: " << failureCount << std::endl;
}

int main() {
    std::cout << "Testing WITHOUT Connection Pool\n";
    std::cout << "==============================\n\n";
    
    std::string host = "localhost";
    unsigned short port = 3306;
    std::string user = "testuser";
    std::string password = "Test@1234";
    std::string database = "testdb";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    int successCount = 0;
    for (int i = 0; i < 1000; ++i) {
        Connection db_conn;
        if (!db_conn.connect(host, port, user, password, database)) {
            std::cout << "Connection failed at iteration " << i << std::endl;
            continue;
        }
        
        std::string sqlQuery = "INSERT INTO user(name, age, sex) VALUES('zhang', 20, 'male')";
        if (db_conn.update(sqlQuery)) {
            successCount++;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start);
    
    std::cout << "Successful operations: " << successCount << "/1000\n";
    std::cout << "Total time: " << duration.count() << " ms\n";
    std::cout << "Average per operation: " << duration.count() / 1000 << " ms\n";
    
    return 0;
}