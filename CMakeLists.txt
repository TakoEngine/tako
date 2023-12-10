cmake_minimum_required(VERSION 3.28)
project("tako")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Magic to enable C++20 modules
set(CMAKE_CXX_EXTENSIONS OFF)
set(CXX_SCAN_FOR_MODULES OFF)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

add_subdirectory("Utils")
add_subdirectory("dependencies/fmt" EXCLUDE_FROM_ALL)
add_subdirectory("dependencies/yaml-cpp" EXCLUDE_FROM_ALL)
add_subdirectory("dependencies/json" EXCLUDE_FROM_ALL)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
if (NOT EMSCRIPTEN)
    add_subdirectory("dependencies/glfw" EXCLUDE_FROM_ALL)
    add_subdirectory("dependencies/assimp" EXCLUDE_FROM_ALL)
endif()
add_subdirectory("Tako")

# Examples
add_subdirectory("Examples/Sandbox")
add_subdirectory("Examples/SandboxVulkan")
add_subdirectory("Examples/JamPhysics2D")
add_subdirectory("Examples/LDtkImport")
