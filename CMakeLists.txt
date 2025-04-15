list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

include(MacroOutOfSourceBuild)
MacroOutOfSourceBuild()

cmake_minimum_required(VERSION 3.10)

project(openEmpires)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)

# Force dynamic runtime (/MD /MDd)
foreach(flag_var
    CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
    string(REPLACE "/MT" "/MD" ${flag_var} "${${flag_var}}")
endforeach()

