function(tako_setup target)
    if (DEFINED EMSCRIPTEN)
        set(CMAKE_EXECUTABLE_SUFFIX ".js" PARENT_SCOPE)
				set_target_properties(${target} PROPERTIES LINK_FLAGS_DEBUG "-g")
				set_target_properties(${target} PROPERTIES LINK_FLAGS "-sEXPORTED_RUNTIME_METHODS=ccall -sALLOW_MEMORY_GROWTH=1")
				set_target_properties(${target} PROPERTIES LINK_FLAGS "-sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency-1")
				target_link_options(${target} INTERFACE "LINKER:-sUSE_PTHREADS=1")
				configure_file(
					${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../Tako/src/platform/web/index.html
					${CMAKE_CURRENT_BINARY_DIR}/index.html
					@ONLY
				)
    endif()
endfunction()

function(tako_assets_dir target dir)

    message(${dir})
    if (DEFINED EMSCRIPTEN)
		set_target_properties(${target} PROPERTIES LINK_FLAGS "--preload-file ${dir}@ -sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency-1 -sEXPORTED_RUNTIME_METHODS=ccall -sALLOW_MEMORY_GROWTH=1")
    else()
        execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${dir} ${CMAKE_CURRENT_BINARY_DIR}/Assets)
    endif()

endfunction()
