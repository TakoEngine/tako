include(tako)
set(EXE LDtkExample)
add_executable(${EXE} "LDtkImport.cpp")
tako_setup(${EXE})
target_link_libraries(${EXE} PRIVATE tako)

tako_assets_dir("${CMAKE_CURRENT_SOURCE_DIR}/Assets")
