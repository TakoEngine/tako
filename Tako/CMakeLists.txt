include(tako)
set(TAKO_DLL "tako")
add_library(${TAKO_DLL}
	#"src/PixelArtDrawer.cpp"
	"src/AudioImpl.cpp"
)

target_sources(
	${TAKO_DLL} PUBLIC
	FILE_SET HEADERS FILES
	"include/Log.hpp"
	"include/Utility.hpp"
	"include/NumberTypes.hpp"
	"include/Texture.hpp"
	#"include/PixelArtDrawer.hpp"
	"include/Reflection.hpp"
	"include/Tako.hpp"
	"include/Sprite.hpp"
	"include/Entity.hpp"
	"include/Timer.hpp"
	"include/GraphicsAPI.hpp"
	"include/IGraphicsContext.hpp"
	"include/VertexBuffer.hpp"
	"include/Pipeline.hpp"
	"include/GameConfig.hpp"
)
target_sources(
	${TAKO_DLL} PUBLIC
	FILE_SET CXX_MODULES FILES
	"src/Window.cppm"
	"src/ECS/World.cppm"
	"src/NumberTypes.cppm"
	"src/Math.cppm"
	"src/Bitmap.cppm"
	"src/Font.cppm"
	"src/Event.cppm"
	"src/Input.cppm"
	"src/EntryPoint.cppm"
	#"src/Jam/PlatformerPhysics2D.cppm"
	"src/Jam/TileMap.cppm"
	"src/Jam/LDtkImporter.cppm"
	"src/SmallVec.cppm"
	"src/Application.cppm"
	"src/FileSystem.cppm"
	"src/Resources.cppm"
	"src/Reflection.cppm"
	"src/YAMLSerialization.cppm"
	"src/Audio.cppm"
	"src/Assets.cppm"
	"src/GraphicsContext.cppm"
	"src/Renderer3D.cppm"
	"src/JobSystem.cppm"
	"src/StringView.cppm"
	"src/HandleVec.cppm"
	"src/Hash.cppm"
	"src/Allocators/Allocator.cppm"
	"src/Allocators/FreeListAllocator.cppm"
	"src/Allocators/LinearAllocator.cppm"
	"src/Allocators/PoolAllocator.cppm"
	"src/Allocators/CachePoolAllocator.cppm"
)

# Workaround for some clang versions crashing when having debug info for JobSystem.cppm
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set_source_files_properties(src/JobSystem.cppm PROPERTIES COMPILE_FLAGS "-g0")
endif()

option (TAKO_FORCE_LOG "Log even in release builds" OFF)
if (TAKO_FORCE_LOG)
	target_compile_definitions(${TAKO_DLL} PUBLIC TAKO_FORCE_LOG)
endif()

function(set_feature define val)
	set(${define} ${val} PARENT_SCOPE)
	if(val)
		#add_compile_definitions(${define})
		#add_definitions(-D${define})
		target_compile_definitions(${TAKO_DLL} PUBLIC ${define})
	endif()
endfunction()

if (WIN32)
set_feature(TAKO_WINDOWS true)
set_feature(TAKO_WIN32 false)
set_feature(TAKO_OPENGL false)
set_feature(TAKO_WEBGPU true)
set_feature(TAKO_VULKAN false)
set_feature(TAKO_GLAD false)
set_feature(TAKO_GLFW true)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set_feature(TAKO_EDITOR true)
	set_feature(TAKO_IMGUI true)
else()
	set_feature(TAKO_EDITOR false)
endif()
endif(WIN32)

if (EMSCRIPTEN)
set_feature(TAKO_EMSCRIPTEN true)
set_feature(TAKO_OPENGL false)
set_feature(TAKO_WEBGPU true)
set_feature(TAKO_VULKAN false)
set_feature(TAKO_EDITOR false)
set_feature(TAKO_IMGUI false)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set_feature(TAKO_MAC true)
elseif(UNIX AND NOT APPLE)
set_feature(TAKO_LINUX true)
endif()

if (TAKO_MAC OR TAKO_LINUX)
set_feature(TAKO_OPENGL false)
set_feature(TAKO_VULKAN false)
set_feature(TAKO_WEBGPU true)
set_feature(TAKO_GLFW true)
set_feature(TAKO_GLAD true)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set_feature(TAKO_EDITOR true)
	set_feature(TAKO_IMGUI true)
else()
	set_feature(TAKO_EDITOR false)
endif()
endif()

if (WIN32)
target_sources(${TAKO_DLL} PRIVATE
	"src/platform/win32/Main.cppm"
	#"src/platform/win32/Window.cpp"
	"src/platform/win32/Log.cpp"
	"include/WinUtility.hpp"
	"src/platform/win32/WinUtility.cpp"
	#"src/platform/default/Timer.cpp"
	PUBLIC FILE_SET CXX_MODULES FILES
	"src/platform/default/FileSystem.cppm"
)
elseif (EMSCRIPTEN)
target_sources(${TAKO_DLL} PRIVATE
	"src/platform/web/Window.cpp"
	"src/platform/web/Log.cpp"
	#"src/platform/web/Timer.cpp"
	"src/platform/default/Timer.cpp" # Temporary, as emscripten_get_now() seems to be broken
	"src/platform/default/Main.cpp"
	PUBLIC FILE_SET CXX_MODULES FILES
	"src/platform/web/FileSystem.cppm"
	)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
target_sources(${TAKO_DLL} PRIVATE
		"src/platform/default/Main.cpp"
		"src/platform/default/Log.cpp"
		PUBLIC FILE_SET CXX_MODULES FILES
		"src/platform/default/FileSystem.cppm"
		)
else()
target_sources(${TAKO_DLL} PRIVATE
		"src/platform/default/Main.cpp"
		"src/platform/default/Log.cpp"
		PUBLIC FILE_SET CXX_MODULES FILES
		"src/platform/default/FileSystem.cppm")
endif()

if (TAKO_OPENGL)
target_sources(${TAKO_DLL} PRIVATE
	"src/platform/opengl/OpenGLPixelArtDrawer.hpp"
	"src/platform/opengl/OpenGLPixelArtDrawer.cpp"
	"src/platform/opengl/OpenGLSprite.hpp"
	"src/platform/opengl/OpenGLSprite.cpp"
	"src/platform/opengl/OpenGLContext.hpp"
	"src/platform/opengl/OpenGLContext.cpp"
)
target_include_directories(${TAKO_DLL} PUBLIC "src/platform/opengl")

	if (TAKO_GLAD)
	target_sources(${TAKO_DLL} PRIVATE
		"src/platform/opengl/glad.c"
		"src/platform/opengl/glad/glad.h"
		"src/platform/opengl/KHR/khrplatform.h"
	)
	else()
		find_package(OpenGL REQUIRED)
		include_directories(${OPENGL_INCLUDE_DIR})
		target_include_directories(${TAKO_DLL} PUBLIC ${OPENGL_INCLUDE_DIR})
	endif(TAKO_GLAD)

function(add_shader filename)
	SET(SHADER_IN ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${filename})
	SET(SHADER_OUT ${CMAKE_BINARY_DIR}/shader/${filename})
	add_custom_command(
		OUTPUT ${SHADER_OUT}
		COMMAND string_literal ${SHADER_IN} ${SHADER_OUT}
		DEPENDS ${SHADER_IN}
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		COMMENT "Preprocessing ${filename}"
	)
	target_sources(${TAKO_DLL} PRIVATE ${SHADER_OUT})
endfunction()

#add_shader("quad.vert.glsl")
#add_shader("quad.frag.glsl")
#add_shader("image.vert.glsl")
#add_shader("image.frag.glsl")

	if (EMSCRIPTEN)
		target_link_libraries(${TAKO_DLL} PRIVATE GLESv2)
	endif()
endif(TAKO_OPENGL)

if (TAKO_VULKAN)
target_compile_definitions(${TAKO_DLL} PUBLIC "GLFW_INCLUDE_VULKAN")
target_sources(${TAKO_DLL} PRIVATE
	"src/platform/vulkan/VulkanContext.hpp"
	"src/platform/vulkan/VulkanContext.cpp"
)
target_include_directories(${TAKO_DLL} PUBLIC "src/platform/vulkan")
#configure_file("shaders/shader.vert.spv" ${CMAKE_BINARY_DIR}/shaders/shader.vert.spv COPYONLY)
#configure_file("shaders/shader.frag.spv" ${CMAKE_BINARY_DIR}/shaders/shader.frag.spv COPYONLY)


find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

function(compile_shader file)
	get_filename_component(filename ${file} NAME_WLE)
	SET(SHADER_IN ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${file})
	SET(SHADER_OUT ${CMAKE_BINARY_DIR}/shader/${filename}.spv)
	add_custom_command(
		OUTPUT ${SHADER_OUT}
		COMMAND ${GLSL_VALIDATOR} -V ${SHADER_IN} -o ${SHADER_OUT}
		DEPENDS ${SHADER_IN}
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		COMMENT "${GLSL_VALIDATOR} -V ${SHADER_IN} -o ${SHADER_OUT}"
	)
	target_sources(${TAKO_DLL} PRIVATE ${SHADER_OUT})
endfunction()

compile_shader("shader.vert.glsl")
compile_shader("shader.frag.glsl")

endif(TAKO_VULKAN)

if (TAKO_WEBGPU)
	add_subdirectory("src/platform/webgpu")
endif()

if (TAKO_GLFW)
target_sources(${TAKO_DLL} PRIVATE
	"src/platform/glfw/Window.cpp"
	"src/platform/glfw/Timer.cpp"
)
target_include_directories(${TAKO_DLL} PUBLIC "src/platform/glfw")
target_compile_definitions(${TAKO_DLL} PUBLIC GLFW_INCLUDE_NONE)
endif()


#add_library(${TAKO_DLL} ${SOURCE_FILES})
target_include_directories(${TAKO_DLL} PUBLIC "include")
target_include_directories(${TAKO_DLL} PUBLIC "${PROJECT_SOURCE_DIR}/dependencies/stb")
target_include_directories(${TAKO_DLL} PUBLIC "${PROJECT_SOURCE_DIR}/dependencies/dr_libs")
target_include_directories(${TAKO_DLL} PUBLIC "${PROJECT_SOURCE_DIR}/dependencies/tinyobjloader")
target_include_directories(${TAKO_DLL} PUBLIC "${PROJECT_SOURCE_DIR}/dependencies/json/include")
target_include_directories(${TAKO_DLL} PUBLIC "${CMAKE_BINARY_DIR}/shader")

# target_link_libraries(${TAKO_DLL} PUBLIC fastgltf_module)
# Use non module import for fastgltf since C++ modules have problems with importing std includes in different modules
target_include_directories(${TAKO_DLL} PUBLIC "${PROJECT_SOURCE_DIR}/dependencies/fastgltf/include")
target_link_libraries(${TAKO_DLL} PUBLIC fastgltf)

if (NOT EMSCRIPTEN)
	#target_include_directories(${TAKO_DLL} PUBLIC "${PROJECT_SOURCE_DIR}/dependencies/assimp/include")
	#target_link_libraries(${TAKO_DLL} PUBLIC assimp)
endif()


#MiniAudio
target_include_directories(${TAKO_DLL} PUBLIC "${PROJECT_SOURCE_DIR}/dependencies/miniaudio")

FetchContent_MakeAvailable(yaml-cpp)
target_link_libraries(${TAKO_DLL} PUBLIC yaml-cpp)
target_link_libraries(${TAKO_DLL} PRIVATE nlohmann_json::nlohmann_json)

#imgui
if (TAKO_IMGUI)
	add_library(imgui STATIC
		"${PROJECT_SOURCE_DIR}/dependencies/imgui/imgui.h"
		"${PROJECT_SOURCE_DIR}/dependencies/imgui/imgui.cpp"
		"${PROJECT_SOURCE_DIR}/dependencies/imgui/imgui_draw.cpp"
		"${PROJECT_SOURCE_DIR}/dependencies/imgui/imgui_tables.cpp"
		"${PROJECT_SOURCE_DIR}/dependencies/imgui/imgui_widgets.cpp"
		"${PROJECT_SOURCE_DIR}/dependencies/imgui/imgui_internal.h"
		"${PROJECT_SOURCE_DIR}/dependencies/imgui/imstb_rectpack.h"
		"${PROJECT_SOURCE_DIR}/dependencies/imgui/imstb_textedit.h"
		"${PROJECT_SOURCE_DIR}/dependencies/imgui/imstb_truetype.h"
		"${PROJECT_SOURCE_DIR}/dependencies/imgui/imgui_demo.cpp"
		"${PROJECT_SOURCE_DIR}/dependencies/imgui/misc/cpp/imgui_stdlib.h"
		"${PROJECT_SOURCE_DIR}/dependencies/imgui/misc/cpp/imgui_stdlib.cpp"
	)
	target_compile_definitions(imgui PUBLIC IMGUI_IMPL_WEBGPU_BACKEND_DAWN)


	if (TAKO_GLFW)
		target_sources(imgui PUBLIC
			"${PROJECT_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_glfw.h"
			"${PROJECT_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_glfw.cpp"
		)
	elseif (TAKO_WIN32)
		target_sources(imgui PUBLIC
			"${PROJECT_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_win32.h"
			"${PROJECT_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_win32.cpp"
		)
	endif()

	if (TAKO_OPENGL)
		target_sources(imgui PUBLIC
			"${PROJECT_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_opengl3.h"
			"${PROJECT_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_opengl3.cpp"
		)
	endif()
	if (TAKO_WEBGPU)
		target_sources(imgui PUBLIC
			"${PROJECT_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_wgpu.h"
			"${PROJECT_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_wgpu.cpp"
		)
	endif()
	target_include_directories(imgui PUBLIC "${PROJECT_SOURCE_DIR}/dependencies/imgui")
	target_include_directories(imgui PUBLIC "${PROJECT_SOURCE_DIR}/dependencies/imgui/backends")
	target_include_directories(imgui PUBLIC "${PROJECT_SOURCE_DIR}/dependencies/imgui/misc/cpp")
	target_link_libraries(imgui PUBLIC glfw webgpu_dawn)
	target_link_libraries(${TAKO_DLL} PUBLIC imgui)
endif()

add_subdirectory("src/RmlUi")
add_subdirectory("src/CSG")

if (TAKO_GLFW)
	target_link_libraries(${TAKO_DLL} PUBLIC glfw)
endif()

if (TAKO_VULKAN)
find_package(Vulkan REQUIRED)
target_include_directories(${TAKO_DLL} PUBLIC ${Vulkan_INCLUDE_DIRS})
if (TAKO_WIN32)
target_compile_definitions(${TAKO_DLL} PRIVATE "VK_USE_PLATFORM_WIN32_KHR")
endif()
target_link_libraries(${TAKO_DLL} PRIVATE ${Vulkan_LIBRARIES})
endif(TAKO_VULKAN)

if (TAKO_EDITOR)
target_sources(${TAKO_DLL} PRIVATE
	"Editor/include/FileWatcher.hpp"
)
target_include_directories(${TAKO_DLL} PUBLIC "Editor/include")
endif(TAKO_EDITOR)


target_link_libraries(${TAKO_DLL} PUBLIC fmt)

FetchContent_MakeAvailable(rapidhash)
FetchContent_GetProperties(rapidhash)
add_library(rapidhash INTERFACE)
target_include_directories(rapidhash INTERFACE "${rapidhash_SOURCE_DIR}")
add_library(rapidhash::rapidhash ALIAS rapidhash)

target_link_libraries(${TAKO_DLL} PRIVATE rapidhash)

add_executable(ECSBench "test/ECSBench.cpp")
target_link_libraries(ECSBench PRIVATE tako)
