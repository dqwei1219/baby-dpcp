#include <iostream>
#include <chrono>
#include "CommonConnectionPool.h"

void testConcurrentWithPool() {
    std::cout << "\n=== Concurrent Test WITH Pool ===" << std::endl;
    
    const int numThreads = 10;
    const int operationsPerThread = 100;
    std::atomic<int> failureCount{0};
    
    auto& pool = ConnectionPool::getConnectionPool();
    
    auto threadFunc = [&pool, &failureCount]() {
        for (int i = 0; i < operationsPerThread; ++i) {
            auto conn = pool.getConnection();
            if (!conn) {
                failureCount++;
                continue;
            }
            
            std::string sqlQuery = "INSERT INTO user(name, age, sex) VALUES('zhao', 35, 'female')";
            conn->update(sqlQuery);
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
    std::cout << "Testing WITH Connection Pool\n";
    std::cout << "===========================\n\n";

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
    return 0;
}