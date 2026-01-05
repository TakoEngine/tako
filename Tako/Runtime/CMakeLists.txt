add_library(${TAKO_RUNTIME})

target_sources(
	${TAKO_RUNTIME} PUBLIC
	FILE_SET CXX_MODULES FILES
	"src/EntryPoint.cppm"
)

if (WIN32)
	target_sources(${TAKO_RUNTIME} PRIVATE
		"src/platform/win32/Main.cppm"
	)
else()
	target_sources(${TAKO_RUNTIME} PRIVATE
		"src/platform/default/Main.cpp"
	)
endif()
target_link_libraries(${TAKO_RUNTIME} PUBLIC ${TAKO_DLL})
