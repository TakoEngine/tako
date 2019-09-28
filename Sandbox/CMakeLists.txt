add_executable(sandbox "Sandbox.cpp")
if (DEFINED EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
  set_target_properties(sandbox PROPERTIES LINK_FLAGS "-s USE_GLFW=3 --emrun")
endif()
target_link_libraries(sandbox PRIVATE tako)

configure_file("./tree.png" "./tree.png" COPYONLY)
