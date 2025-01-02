target_sources(${TAKO_DLL} PRIVATE
	PUBLIC FILE_SET CXX_MODULES FILES
	"WebGPUContext.cppm"
)

if (NOT EMSCRIPTEN)
	#find_package(Dawn REQUIRED)
	target_link_libraries(${TAKO_DLL} PRIVATE webgpu_dawn)
endif()

if (TAKO_GLFW)
target_link_libraries(${TAKO_DLL} PRIVATE dawn_glfw)
endif()
