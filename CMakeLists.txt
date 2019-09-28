cmake_minimum_required(VERSION 3.7)
project("tako")

set(CMAKE_CXX_STANDARD 17)

add_subdirectory("Utils")
add_subdirectory("dependencies/fmt" EXCLUDE_FROM_ALL)
add_subdirectory("Tako")
add_subdirectory("Sandbox")
