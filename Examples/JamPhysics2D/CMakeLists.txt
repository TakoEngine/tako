include(tako)
add_executable(jamPhysics2D "JamPhysics2D.cpp")
tako_setup(jamPhysics2D)
target_link_libraries(jamPhysics2D PRIVATE tako)
if (DEFINED EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

tako_assets_dir("${CMAKE_CURRENT_SOURCE_DIR}/Assets/")
