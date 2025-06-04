list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

# Prevent in-source builds
include(MacroOutOfSourceBuild)
MacroOutOfSourceBuild()

cmake_minimum_required(VERSION 3.10)
project(openEmpires CXX)

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
# Explicitly setting the flags to force static runtime. CMAKE_MSVC_RUNTIME_LIBRARY didn't work.
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /W1") #  Temporarilty lowering to make the build faster
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /W4")
endif()

# Outputs will be in build/bin directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)
add_subdirectory(tests)

include(ClangFormatUtils)
# Define patterns and ignore file
set(GLOB_PATTERNS "src/*.cpp" "src/*.h")
set(IGNORE_FILE "${CMAKE_SOURCE_DIR}/.clang-format-ignore")

# Collect format files
collect_format_files(SOURCE_FILES_TO_FORMAT "${CMAKE_SOURCE_DIR}" "${GLOB_PATTERNS}" "${IGNORE_FILE}")

# Find clang-format
find_program(CLANG_FORMAT_BIN clang-format)

# Check if clang-format was found
if(CLANG_FORMAT_BIN)
    # Add a custom target to run clang-format
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_BIN}
        -i
        -style=file
        ${SOURCE_FILES_TO_FORMAT}
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


string(TIMESTAMP PROJECT_VERSION_BUILD "%Y%m%d")
# Configure Version.h
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/generated)
configure_file(
    ${CMAKE_SOURCE_DIR}/src/cpp/Version.h.in
    ${CMAKE_BINARY_DIR}/generated/Version.h
    @ONLY
)
set(GENERATED_DIR ${CMAKE_BINARY_DIR}/generated CACHE INTERNAL "")