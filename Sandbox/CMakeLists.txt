add_executable(sandbox WIN32 "Sandbox.cpp")
target_link_libraries(sandbox PRIVATE tako)

configure_file("./tree.png" "./tree.png" COPYONLY)