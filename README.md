# DBCP - Database Connection Pool
A lightweight, database connection pooling library implemented in C++.

## Overview
DBCP solves a critical performance bottleneck in database-driven applications by efficiently managing database connections. This library provides an optimized connection pooling mechanism that significantly reduces the overhead associated with establishing database connections.

### The Problem

Creating a new database connection for each user request can be resource-intensive and time-consuming:

- A single connection setup can take hundreds of milliseconds to seconds
- The actual database transaction might only take milliseconds
- For applications with many simultaneous users, creating individual connections becomes impractical
- Most users only require a database connection during request processing, which is a small percentage of their total session time

### Connection Establishment Overhead

Each new database connection incurs multiple expensive operations:

1. TCP 3-way handshake
2. SQL authentication
3. SQL session initialization
4. TCP 4-way handshake (when closing)

DBCP eliminates these overheads by maintaining a pool of reusable connections.

## Key Features

- **Optimal Resource Management**: Maintains a pool of pre-established database connections
- **Dynamic Scaling**: Automatically adjusts the connection pool size based on demand
- **Resource Conservation**: Closes idle connections to free system resources when demand decreases
- **Configurable Behavior**: Customizable pool size, timeout settings, and connection parameters
- **Thread-Safe Design**: Safely handles concurrent connection requests

## Core Functionality

DBCP provides four essential configuration parameters:

1. **`init_size`**: Initial number of connections created when the pool is initialized
2. **`max_size`**: Maximum number of connections the pool can grow to accommodate increased demand
3. **`max_idle_time`**: Maximum time (in seconds) an unused connection remains in the pool before being closed
4. **`connection_timeout`**: Maximum time (in milliseconds) a request will wait for an available connection before timing out

## Implementation

The project consists of two main components:

1. **`ConnectionPool.cpp`**: Implements the connection pooling logic and management functions
2. **`Connection.cpp`**: Provides the SQL CRUD (Create, Read, Update, Delete) operation interfaces

## Usage Example

```cpp
#include "ConnectionPool.h"

int main() {
    // Initialize the connection pool
    ConnectionPool* pool = ConnectionPool::getInstance();
    pool->init("localhost", 3306, "username", "password", "dbname", 5, 20, 100, 1000);
    
    // Get a connection from the pool
    Connection* conn = pool->getConnection();
    
    // Use the connection for database operations
    conn->update("UPDATE users SET status = 1 WHERE id = 100");
    
    // Return the connection to the pool
    pool->releaseConnection(conn);
    
    return 0;
}
```

# DBCP System Architecture Diagram
```
┌─────────────┐                                                        ┌────────────────┐
│             │                                                        │                │
│    user1    │─────────────────┐                                      │                │
│             │    request      │                                      │                │
└─────────────┘                 │                                      │                │
                                │                                      │                │
┌─────────────┐                 │                                      │                │
│             │                 │                                      │                │
│    user2    │─────────────────┼────────► ┌───────────────┐           │   MySQL Server │
│             │    request      │          │  Server App   │           │                │
└─────────────┘                 │          │ (Multi-Thread)│           │                │
                                │          │               │ ─────────►│                |
┌─────────────┐                 │          │  MySQL Client │           │                │
│             │                 │          │               │           │                │
│    user3    │─────────────────┘          └───────┬───────┘           │                │
│             │    request                         │                   │                │
└─────────────┘                                    │                   └────────────────┘
                                                   │
                                                   │
                                  ┌────────────────▼────────────────────────┐
                                  │                                         │
                                  │  Connection1 Conn2 Conn3 Conn4 Conn5 ...│
                                  │  ─────────────────────────────────────  │
                                  │  Connection Pool Queue                  │
                                  │                                         │
                                  └─────────────────────────────────────────┘
```

## Building and Installation

```bash
# Clone the repository
git clone https://github.com/dqwei1219/baby-dpcp 
cd baby-dbcp

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make

# Run tests
make test

# Install library
make install
```

## Performance Considerations and Testing

Performance score