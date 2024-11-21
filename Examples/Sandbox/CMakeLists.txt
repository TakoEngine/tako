include(tako)
add_executable(Sandbox "Sandbox.cpp")
tako_setup(Sandbox)
target_link_libraries(Sandbox PRIVATE tako)

tako_assets_dir(Sandbox "${CMAKE_CURRENT_SOURCE_DIR}/Assets")

