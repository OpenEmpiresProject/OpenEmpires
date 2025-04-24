list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

# Prevent in-source builds
include(MacroOutOfSourceBuild)
MacroOutOfSourceBuild()

cmake_minimum_required(VERSION 3.10)
project(openEmpires)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W1") #  Temporarilty lowering to make the build faster
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W4")
endif()

# Outputs will be in build/bin directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)
add_subdirectory(tests)

# Find all source/header files
file(GLOB_RECURSE ALL_SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.h"
    "${CMAKE_SOURCE_DIR}/include/*.h"
)

# Find clang-format
find_program(CLANG_FORMAT_BIN clang-format)

# Check if clang-format was found
if(CLANG_FORMAT_BIN)
    # Add a custom target to run clang-format
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_BIN}
        -i
        -style=file
        ${ALL_SOURCE_FILES}
        COMMENT "Running clang-format on source files"
    )
else()
    message(WARNING "clang-format not found!")
endif()

# Find Cppcheck
find_program(CPPCHECK_EXECUTABLE cppcheck)

# Check if Cppcheck was found
if(CPPCHECK_EXECUTABLE)
    set(CPPCHECK_REPORT_FILE "${CMAKE_BINARY_DIR}/cppcheck_report.xml")
    # Add a custom target to run Cppcheck
    add_custom_target(
        cppcheck
        COMMAND ${CPPCHECK_EXECUTABLE} 
            --enable=all
            --suppress=missingIncludeSystem
            --suppress=missingInclude
            "${CMAKE_SOURCE_DIR}/src"
            --output-file=${CPPCHECK_REPORT_FILE}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running Cppcheck"
    )

    add_custom_command(
        TARGET cppcheck
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Cppcheck analysis completed. Report saved to: ${CPPCHECK_REPORT_FILE}"
    )
else()
  message(WARNING "Cppcheck not found!")
endif()
