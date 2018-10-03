cmake_minimum_required(VERSION 3.6)
project("tako")

set(CMAKE_CXX_STANDARD 17)

set(EXECUTABLE_NAME "tako")
include_directories("include")
include_directories("dependencies/stb")
include_directories("C:\\VulkanSDK\\1.1.82.1\\Include")
link_directories("C:\\VulkanSDK\\1.1.82.1\\Lib")
set(SOURCE_FILES
	"src/main.cpp"
	"include/Window.hpp"
	"src/Window.cpp"
	"include/Log.hpp"
	"src/Log.cpp"
	"include/Utility.hpp"
	"include/FileSystem.hpp"
	"src/FileSystem.cpp"
	"include/WinUtility.hpp"
	"include/NumberTypes.hpp"
	"include/Math.hpp"
	"include/Bitmap.hpp"
	"src/Bitmap.cpp")

add_subdirectory("dependencies/fmt" EXCLUDE_FROM_ALL)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
add_executable(${EXECUTABLE_NAME} WIN32 ${SOURCE_FILES})
target_link_libraries(${EXECUTABLE_NAME} PRIVATE fmt)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE vulkan-1)