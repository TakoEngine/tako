cmake_minimum_required(VERSION 3.30)
project("tako")

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Magic to enable C++20 modules
set(CMAKE_CXX_EXTENSIONS OFF)
#set(CXX_SCAN_FOR_MODULES OFF)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

if(EMSCRIPTEN)
    set(EMSCRIPTEN_PTHREADS_COMPILER_FLAGS "-pthread")
    set(EMSCRIPTEN_PTHREADS_LINKER_FLAGS "${EMSCRIPTEN_PTHREADS_COMPILER_FLAGS}")

    string(APPEND CMAKE_C_FLAGS " ${EMSCRIPTEN_PTHREADS_COMPILER_FLAGS}")
    string(APPEND CMAKE_CXX_FLAGS " ${EMSCRIPTEN_PTHREADS_COMPILER_FLAGS}")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " ${EMSCRIPTEN_PTHREADS_LINKER_FLAGS}")
endif()

add_subdirectory("Utils")
add_subdirectory("dependencies/fmt" EXCLUDE_FROM_ALL)
add_subdirectory("dependencies/json" EXCLUDE_FROM_ALL)

FetchContent_Declare(
	yaml-cpp
	GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
	GIT_TAG 28f93bdec6387d42332220afa9558060c8016795
)

set(FASTGLTF_COMPILE_AS_CPP20 ON CACHE BOOL "" FORCE)
#set(FASTGLTF_ENABLE_CPP_MODULES ON CACHE BOOL "" FORCE)
add_subdirectory("dependencies/fastgltf" EXCLUDE_FROM_ALL)
FetchContent_Declare(
	freetype
	GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
	GIT_TAG 42608f77f20749dd6ddc9e0536788eaad70ea4b5 # 2.13.3
	FIND_PACKAGE_ARGS NAMES Freetype
)
#if (NOT EMSCRIPTEN)
	FetchContent_MakeAvailable(freetype)
	if (NOT TARGET Freetype::Freetype)
		add_library(Freetype::Freetype ALIAS freetype)
	endif()
#endif()
add_subdirectory("${PROJECT_SOURCE_DIR}/dependencies/RmlUi" EXCLUDE_FROM_ALL)

if (NOT EMSCRIPTEN)
	set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
	add_subdirectory("dependencies/glfw" EXCLUDE_FROM_ALL)

	#add_subdirectory("dependencies/assimp" EXCLUDE_FROM_ALL)
endif()

FetchContent_Declare(
	dawn
	URL https://github.com/google/dawn/archive/a4695b94d4191bf496707dc121b180d4b81ee6b4.zip
	URL_HASH SHA256=b2d4938bac76a5affb550c915c70ac82ab9b6aec574bae05eee93dc03992a825
	FIND_PACKAGE_ARGS NAMES Dawn
)

FetchContent_Declare(
	manifold
	GIT_REPOSITORY https://github.com/elalish/manifold.git
	GIT_TAG 6754a3cf7cd3a16940efa4a9751cb63b0f49abf3 # 3.2.0
)

FetchContent_Declare(
	rapidhash
	GIT_REPOSITORY https://github.com/Nicoshev/rapidhash.git
	GIT_TAG abc8d971489440a13e18db37d09b5d6e4484737b
)

add_subdirectory("Tako")

# Examples
add_subdirectory("Examples/Sandbox")
add_subdirectory("Examples/Sandbox3D")
add_subdirectory("Examples/JamPhysics2D")
add_subdirectory("Examples/LDtkImport")
