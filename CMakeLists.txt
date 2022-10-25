cmake_minimum_required(VERSION 3.11)
project(tameshi)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)

add_executable(tameshi
    source/udpsocket.cpp
    source/udpsocket.h
    source/main.cpp)

target_link_libraries(tameshi -pthread)
