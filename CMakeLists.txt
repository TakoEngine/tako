cmake_minimum_required(VERSION 3.26)
project("tako")

set(CMAKE_CXX_STANDARD 20)

# Magic to enable C++20 modules
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API 2182bf5c-ef0d-489a-91da-49dbc3090d2a)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)

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
