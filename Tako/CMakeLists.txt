include(tako)
set(TAKO_DLL "tako")

set(SOURCE_FILES
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
	"include/Texture.hpp"
	"include/PixelArtDrawer.hpp"
	"src/platform/opengl/OpenGLPixelArtDrawer.hpp"
	"src/platform/opengl/OpenGLPixelArtDrawer.cpp"
	"include/Texture.hpp"
	"src/platform/opengl/OpenGLTexture.hpp"
	"src/platform/opengl/OpenGLTexture.cpp"
#	"src/VulkanContext.cpp"
    "src/platform/opengl/OpenGLContext.cpp"
	"include/Tako.hpp"
	"include/Event.hpp"
	"src/Event.cpp"
	)

if (WIN32)
set(SOURCE_FILES ${SOURCE_FILES}
	"src/platform/win32/Window.cpp"
	"src/platform/win32/Log.cpp"
	"src/platform/win32/FileSystem.cpp"
	"include/WinUtility.hpp"
	"src/platform/win32/WinUtility.cpp"
)
ENDIF(WIN32)

if (EMSCRIPTEN)
set(SOURCE_FILES ${SOURCE_FILES}
	"src/platform/web/Window.cpp"
	"src/platform/web/Log.cpp"
	"src/platform/web/FileSystem.cpp")
ENDIF(EMSCRIPTEN)

function(add_shader filename)
	SET(SHADER_IN ${CMAKE_SOURCE_DIR}/Tako/shaders/${filename})
	SET(SHADER_OUT ${CMAKE_BINARY_DIR}/shader/${filename})
	add_custom_command(
		OUTPUT ${SHADER_OUT}
		COMMAND string_literal ${SHADER_IN} ${SHADER_OUT}
		DEPENDS ${SHADER_IN}
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		COMMENT "Preprocessing ${filename}"
	)
	set(SOURCE_FILES ${SOURCE_FILES} ${SHADER_OUT} PARENT_SCOPE)
endfunction()

add_shader("quad.vert.glsl")
add_shader("quad.frag.glsl")
add_shader("image.vert.glsl")
add_shader("image.frag.glsl")


add_library(${TAKO_DLL} ${SOURCE_FILES})
target_include_directories(${TAKO_DLL} PUBLIC "include")
target_include_directories(${TAKO_DLL} PUBLIC "${PROJECT_SOURCE_DIR}/dependencies/stb")
target_include_directories(${TAKO_DLL} PUBLIC "${CMAKE_BINARY_DIR}/shader")

#[[ Vulkan
find_package(vulkan REQUIRED)
target_include_directories(${TAKO_DLL} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_compile_definitions(${TAKO_DLL} PRIVATE "VK_USE_PLATFORM_WIN32_KHR")
target_link_libraries(${TAKO_DLL} PRIVATE ${Vulkan_LIBRARIES})
]]

#find_package(glbinding REQUIRED)
#configure_file(${glbinding_DIR}/glbinding.dll "glbinding.dll" COPYONLY)
#target_link_libraries(${TAKO_DLL} PRIVATE glbinding::glbinding)
target_link_libraries(${TAKO_DLL} PRIVATE GLESv2)



target_link_libraries(${TAKO_DLL} PUBLIC fmt)
