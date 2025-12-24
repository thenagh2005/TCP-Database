cmake_minimum_required(VERSION 3.10)
project(cpp-db-backend VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# src
add_library(kvstore src/kv/kvstore.cpp src/kv/zset.cpp)
target_include_directories(kvstore PUBLIC src)

# TCP server executable
add_executable(tcp_server 
    src/main.cpp
    src/net/tcp_server.cpp
)
target_link_libraries(tcp_server PRIVATE kvstore pthread)
target_include_directories(tcp_server PRIVATE src)

# tests (using built-in testing)
enable_testing()
add_executable(test_kv tests/test_kv.cpp)
target_link_libraries(test_kv PRIVATE kvstore)
add_test(NAME KVStoreTest COMMAND test_kv)

add_executable(test_zset tests/test_zset.cpp)
target_link_libraries(test_zset PRIVATE kvstore)
add_test(NAME ZSetTest COMMAND test_zset)