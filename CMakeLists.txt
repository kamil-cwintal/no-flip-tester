cmake_minimum_required (VERSION 3.0.2)
set (CMAKE_CXX_STANDARD 14)

project (no-flip-tester)

file (GLOB SOURCES "src/*.cpp")
add_executable (no-flip-tester ${SOURCES})
