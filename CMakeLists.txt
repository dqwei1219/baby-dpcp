cmake_minimum_required(VERSION 3.10)
project(baby-dbcp)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find MySQL Connector/C++
find_library(MYSQLCPPCONN_LIBRARY mysqlcppconn REQUIRED)

# Add executable
add_executable(baby-dbcp src/main.cc)

# Include directories
target_include_directories(baby-dbcp PRIVATE /usr/include/cppconn)

# Link libraries
target_link_libraries(baby-dbcp ${MYSQLCPPCONN_LIBRARY})