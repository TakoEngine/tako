include(tako)
set(EXE Sandbox3D)
add_executable(${EXE} "Sandbox.cpp")
tako_setup(${EXE})
target_link_libraries(${EXE} PRIVATE tako)

tako_assets_dir(${EXE} "${CMAKE_CURRENT_SOURCE_DIR}/Assets/")
