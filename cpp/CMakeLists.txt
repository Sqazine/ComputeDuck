cmake_minimum_required(VERSION 3.10)

project(ComputeDuck LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB SRC "*.h" "*.cpp")

add_executable(computeduck ${SRC})