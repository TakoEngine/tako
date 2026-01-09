target_sources(${TAKO_DLL} PRIVATE
	PUBLIC FILE_SET CXX_MODULES FILES
	"ImGuiPlugin.cppm"
)

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
