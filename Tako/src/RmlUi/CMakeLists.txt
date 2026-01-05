target_sources(${TAKO_DLL} PRIVATE
	"${PROJECT_SOURCE_DIR}/dependencies/RmlUi/Backends/RmlUi_Platform_GLFW.cpp"
	PUBLIC FILE_SET CXX_MODULES FILES
	"RmlUi.cppm"
	"RmlUiRenderer.cppm"
	"RmlUiSystem.cppm"
	"RmlUiFile.cppm"
)

target_include_directories(${TAKO_DLL} PUBLIC "${PROJECT_SOURCE_DIR}/dependencies/RmlUi/Backends")
target_link_libraries(${TAKO_DLL} PUBLIC RmlUi::RmlUi)
