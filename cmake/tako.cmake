function(tako_setup target)
    if (DEFINED EMSCRIPTEN)
        set(CMAKE_EXECUTABLE_SUFFIX ".js" PARENT_SCOPE)
				#set_target_properties(${target} PROPERTIES LINK_FLAGS "--emrun")
				#set_target_properties(${target} PROPERTIES LINK_FLAGS "--source-map-base http://localhost:8080/")
				#set_target_properties(${target} PROPERTIES LINK_FLAGS "-gsource-map")
				#set_target_properties(${target} PROPERTIES LINK_FLAGS "-g")
				set_target_properties(${target} PROPERTIES LINK_FLAGS "-sEXPORTED_RUNTIME_METHODS=ccall -sALLOW_MEMORY_GROWTH=1")
				configure_file(
					${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../Tako/src/platform/web/index.html
					${CMAKE_CURRENT_BINARY_DIR}/index.html
					@ONLY
				)
    endif()
endfunction()

function(tako_assets_dir dir)

    message(${dir})
    if (DEFINED EMSCRIPTEN)
        #set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "--preload-file ${dir}@" PARENT_SCOPE)
    else()
        execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${dir} ${CMAKE_CURRENT_BINARY_DIR}/Assets)
    endif()

endfunction()
