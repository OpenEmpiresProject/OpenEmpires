list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

# Prevent in-source builds
include(MacroOutOfSourceBuild)
MacroOutOfSourceBuild()

cmake_minimum_required(VERSION 3.10)
project(openEmpires)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

if(CLANG_FORMAT_BIN)
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
