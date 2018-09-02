cmake_minimum_required(VERSION 3.6)
project("tako")

set(CMAKE_CXX_STANDARD 17)

set(EXECUTABLE_NAME "tako")
include_directories("include")
set(SOURCE_FILES "src/main.cpp" "include/Window.hpp" "src/Window.cpp")
add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES})