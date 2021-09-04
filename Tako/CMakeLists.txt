include(tako)
set(TAKO_DLL "tako")
add_library(${TAKO_DLL}
	"include/Window.hpp"
	"include/Log.hpp"
	"include/Utility.hpp"
	"include/FileSystem.hpp"
	"include/NumberTypes.hpp"
	"include/Math.hpp"
	"src/Math.cpp"
	"include/Bitmap.hpp"
	"src/Bitmap.cpp"
	"include/GraphicsContext.hpp"
	"src/GraphicsContext.cpp"
	"include/Texture.hpp"
	"include/PixelArtDrawer.hpp"
	"include/Tako.hpp"
	"include/Event.hpp"
	"src/Event.cpp"
	"include/Input.hpp"
	"src/Input.cpp"
	"include/Sprite.hpp"
	"include/Audio.hpp"
	"src/Audio.cpp"
	"include/Entity.hpp"
	"include/World.hpp"
	"src/World.cpp"
	"include/EntryPoint.hpp"
	"src/EntryPoint.cpp"
	"include/Font.hpp"
	"include/Timer.hpp"
	"include/Resources.hpp"
	"src/Resources.cpp"
	"include/GraphicsAPI.hpp"
	"include/IGraphicsContext.hpp"
	"include/SmallVec.hpp"
	"include/VertexBuffer.hpp"
	"include/Renderer3D.hpp"
	"src/Renderer3D.cpp"
	"include/GameConfig.hpp")

function(set_feature define val)
	set(${define} ${val} PARENT_SCOPE)
	if(val)
		#add_compile_definitions(${define})
		#add_definitions(-D${define})
		target_compile_definitions(${TAKO_DLL} PUBLIC ${define})
	endif()
endfunction()

if (WIN32)
set_feature(TAKO_WIN32 true)
set_feature(TAKO_OPENGL false)
set_feature(TAKO_VULKAN true)
set_feature(TAKO_OPENAL false)
set_feature(TAKO_GLAD true)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set_feature(TAKO_EDITOR false)
else()
	set_feature(TAKO_EDITOR false)
endif()
endif(WIN32)

if (EMSCRIPTEN)
set_feature(TAKO_EMSCRIPTEN true)
set_feature(TAKO_OPENGL true)
set_feature(TAKO_VULKAN false)
set_feature(TAKO_OPENAL true)
set_feature(TAKO_EDITOR false)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set_feature(TAKO_MAC true)
elseif(UNIX AND NOT APPLE)
set_feature(TAKO_LINUX true)
endif()

if (TAKO_MAC OR TAKO_LINUX)
set_feature(TAKO_OPENGL true)
set_feature(TAKO_VULKAN true)
set_feature(TAKO_OPENAL true)
set_feature(TAKO_GLFW true)
set_feature(TAKO_GLAD true)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set_feature(TAKO_EDITOR true)
else()
	set_feature(TAKO_EDITOR false)
endif()
endif()

if (WIN32)
target_sources(${TAKO_DLL} PRIVATE
	"src/platform/win32/Main.cpp"
	"src/platform/win32/Window.cpp"
	"src/platform/win32/Log.cpp"
	"src/platform/win32/FileSystem.cpp"
	"include/WinUtility.hpp"
	"src/platform/win32/WinUtility.cpp"
	"src/platform/default/Timer.cpp"
)
elseif (EMSCRIPTEN)
target_sources(${TAKO_DLL} PRIVATE
	"src/platform/web/Window.cpp"
	"src/platform/web/Log.cpp"
	"src/platform/web/FileSystem.cpp"
	"src/platform/web/Timer.cpp"
	"src/platform/default/Main.cpp")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
target_sources(${TAKO_DLL} PRIVATE
		"src/platform/default/FileSystem.cpp"
		"src/platform/default/Main.cpp"
		"src/platform/default/Log.cpp")
elseif (UNIX AND NOT APPLE)
target_sources(${TAKO_DLL} PRIVATE
		"src/platform/default/FileSystem.cpp"
		"src/platform/default/Main.cpp"
		"src/platform/default/Log.cpp")
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

add_shader("quad.vert.glsl")
add_shader("quad.frag.glsl")
add_shader("image.vert.glsl")
add_shader("image.frag.glsl")

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
endif(TAKO_VULKAN)

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
target_include_directories(${TAKO_DLL} PUBLIC "${PROJECT_SOURCE_DIR}/dependencies/assimp/include")
target_include_directories(${TAKO_DLL} PUBLIC "${CMAKE_BINARY_DIR}/shader")

target_link_libraries(${TAKO_DLL} PUBLIC assimp)

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

if (TAKO_OPENAL)
	if (EMSCRIPTEN)
		target_link_libraries(${TAKO_DLL} PRIVATE openal)
	else()
		find_package(OpenAL REQUIRED)
		target_include_directories(${TAKO_DLL} PUBLIC ${OPENAL_INCLUDE_DIR})
		target_link_libraries(${TAKO_DLL} PRIVATE ${OPENAL_LIBRARY})
	endif()
endif(TAKO_OPENAL)


if (TAKO_EDITOR)
target_sources(${TAKO_DLL} PRIVATE
	"Editor/include/FileWatcher.hpp"
)
target_include_directories(${TAKO_DLL} PUBLIC "Editor/include")
endif(TAKO_EDITOR)


target_link_libraries(${TAKO_DLL} PUBLIC fmt)

add_executable(ECSBench "test/ECSBench.cpp")
target_link_libraries(ECSBench PRIVATE tako)
