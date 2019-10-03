include(tako)
add_executable(sandbox "Sandbox.cpp")
tako_setup(sandbox)
target_link_libraries(sandbox PRIVATE tako)

tako_assets_dir("${CMAKE_CURRENT_SOURCE_DIR}/Assets/@")