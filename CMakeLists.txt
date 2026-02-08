cmake_minimum_required(VERSION 3.16)
project(epoll_sever CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)
add_executable(server 
    src/main.cpp
    src/tcp_server.cpp
)

target_compile_options(server PRIVATE -Wall -Wextra -O2)