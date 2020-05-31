cmake_minimum_required(VERSION 3.14)
project(mail_test)

set(CMAKE_CXX_STANDARD 17)

add_executable(mail_test main.cpp)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")