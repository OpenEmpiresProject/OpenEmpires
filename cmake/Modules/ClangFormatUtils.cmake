function(collect_format_files OUT_VAR BASE_DIR GLOB_PATTERNS IGNORE_FILE)
    # Collect all files based on the glob patterns
    set(all_files "")
    foreach(pattern IN LISTS GLOB_PATTERNS)
        file(GLOB_RECURSE matched_files "${BASE_DIR}/${pattern}")
        list(APPEND all_files ${matched_files})
    endforeach()

    # Read the ignore file
    file(READ "${IGNORE_FILE}" ignore_contents)
    string(REPLACE "\n" ";" ignore_lines "${ignore_contents}")

    # Normalize and strip ignore patterns
    set(ignore_paths "")
    foreach(path IN LISTS ignore_lines)
        string(STRIP "${path}" path)
        if(NOT path STREQUAL "")
            file(TO_CMAKE_PATH "${BASE_DIR}/${path}" abs_ignore_path)
            list(APPEND ignore_paths "${abs_ignore_path}")
        endif()
    endforeach()

    # Filter out ignored files
    set(selected_files "")
    foreach(file ${all_files})
        set(ignore_file FALSE)
        foreach(ignore_path ${ignore_paths})
            if(file MATCHES "^${ignore_path}.*")
                set(ignore_file TRUE)
                break()
            endif()
        endforeach()
        if(NOT ignore_file)
            list(APPEND selected_files "${file}")
        endif()
    endforeach()

    # Return result
    set(${OUT_VAR} "${selected_files}" PARENT_SCOPE)
endfunction()
