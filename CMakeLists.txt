cmake_minimum_required(VERSION 3.23)
project(ChatRoom)

set(CMAKE_CXX_STANDARD 20)

set(Headers Server.h, Client.h)
add_executable(ChatRoom ${PROJECT_BINARY_DIR} / main.cpp Server.cpp Client.cpp)
