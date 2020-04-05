function(tako_setup target)
    if (DEFINED EMSCRIPTEN)
        #set(CMAKE_EXECUTABLE_SUFFIX ".html" PARENT_SCOPE)
        set_target_properties(${target} PROPERTIES LINK_FLAGS "--emrun")
    endif()
endfunction()

function(tako_assets_dir dir)

    message(${dir})
    if (DEFINED EMSCRIPTEN)
        set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "--embed-file ${dir}@" PARENT_SCOPE)
    else()
        execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${dir} ${CMAKE_CURRENT_BINARY_DIR}/Assets)
    endif()

endfunction()