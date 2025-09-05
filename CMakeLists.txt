cmake_minimum_required(VERSION 3.12)
project(cpp_reactor)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_executable(cpp_reactor src/main.cc
            src/TcpConn.cc
            src/TcpServer.cc
    )
endif ()