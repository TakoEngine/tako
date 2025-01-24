target_sources(${TAKO_DLL} PRIVATE
	PUBLIC FILE_SET CXX_MODULES FILES
	"WebGPUContext.cppm"
)

include(FetchContent)

if (NOT EMSCRIPTEN)
	#find_package(Dawn REQUIRED)
	set(DAWN_FETCH_DEPENDENCIES ON CACHE BOOL "" FORCE)
	set(DAWN_USE_WAYLAND ON CACHE BOOL "" FORCE)
	set(DAWN_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
	FetchContent_MakeAvailable(dawn)
	target_link_libraries(${TAKO_DLL} PRIVATE webgpu_dawn)
endif()

if (TAKO_GLFW)
target_link_libraries(${TAKO_DLL} PRIVATE dawn_glfw)
endif()
