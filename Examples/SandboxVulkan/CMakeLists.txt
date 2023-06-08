include(tako)
add_executable(sandboxVulkan "Sandbox.cppm")
tako_setup(sandboxVulkan)
target_link_libraries(sandboxVulkan PRIVATE tako)
if (DEFINED EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

tako_assets_dir("${CMAKE_CURRENT_SOURCE_DIR}/Assets/")
