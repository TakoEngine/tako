include(tako)
add_executable(Sandbox "Sandbox.cpp")
tako_setup(Sandbox)
target_link_libraries(Sandbox PRIVATE tako)
if (DEFINED EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

tako_assets_dir("${CMAKE_CURRENT_SOURCE_DIR}/Assets")

