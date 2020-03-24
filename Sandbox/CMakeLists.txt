include(tako)
add_executable(sandbox WIN32 "Sandbox.cpp")
tako_setup(sandbox)
target_link_libraries(sandbox PRIVATE tako)
if (DEFINED EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

tako_assets_dir("${CMAKE_CURRENT_SOURCE_DIR}/Assets/")
