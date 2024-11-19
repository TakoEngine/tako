include(tako)
add_executable(JamPhysics2D "JamPhysics2D.cpp")
tako_setup(JamPhysics2D)
target_link_libraries(JamPhysics2D PRIVATE tako)
if (DEFINED EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

#tako_assets_dir(JamPhysics2D "${CMAKE_CURRENT_SOURCE_DIR}/Assets/")
