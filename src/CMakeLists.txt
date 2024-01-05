cmake_minimum_required(VERSION 3.16.3)
project(cp)

set(CMAKE_CXX_STANDARD 20)

add_executable(server server.cpp)
add_executable(client client.cpp)

target_link_libraries(server zmq)
target_link_libraries(client zmq)