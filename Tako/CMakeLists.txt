set(TAKO_DLL "tako")

set(SOURCE_FILES
	"include/Window.hpp"
	"src/Window.cpp"
	"include/Log.hpp"
	"src/Log.cpp"
	"include/Utility.hpp"
	"include/FileSystem.hpp"
	"src/FileSystem.cpp"
	"include/WinUtility.hpp"
	"src/WinUtility.cpp"
	"include/NumberTypes.hpp"
	"include/Math.hpp"
	"src/Math.cpp"
	"include/Bitmap.hpp"
	"src/Bitmap.cpp"
	"include/GraphicsContext.hpp"
	"src/VulkanContext.cpp"
	"include/Tako.hpp")

add_library(${TAKO_DLL} ${SOURCE_FILES})
target_include_directories(${TAKO_DLL} PUBLIC "include")
target_include_directories(${TAKO_DLL} PUBLIC "${PROJECT_SOURCE_DIR}/dependencies/stb")

find_package(vulkan REQUIRED)
target_include_directories(${TAKO_DLL} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_compile_definitions(${TAKO_DLL} PRIVATE "VK_USE_PLATFORM_WIN32_KHR")
target_link_libraries(${TAKO_DLL} PUBLIC fmt)
target_link_libraries(${TAKO_DLL} PRIVATE ${Vulkan_LIBRARIES})