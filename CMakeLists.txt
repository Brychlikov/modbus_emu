cmake_minimum_required(VERSION 3.15)
project(modbus)

set(CMAKE_CXX_STANDARD 14)

add_executable(modbus
        src/main.cpp)

add_executable(pipetest src/pipetest.cpp)
