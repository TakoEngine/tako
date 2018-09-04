cmake_minimum_required(VERSION 3.6)
project("tako")

set(CMAKE_CXX_STANDARD 17)

set(EXECUTABLE_NAME "tako")
include_directories("include")
set(SOURCE_FILES
	"src/main.cpp"
	"include/Window.hpp"
	"src/Window.cpp"
	"include/Log.hpp"
	"src/Log.cpp"
	"include/Utility.hpp")

add_subdirectory("dependencies/fmt" EXCLUDE_FROM_ALL)
add_executable(${EXECUTABLE_NAME} WIN32 ${SOURCE_FILES})
target_link_libraries(${EXECUTABLE_NAME} PRIVATE fmt)