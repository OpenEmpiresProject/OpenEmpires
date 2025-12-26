
find_package(GTest CONFIG REQUIRED)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_executable(openEmpiresTests ${TEST_SOURCES})

# Without setting this property, the working directory will be build/src/cpp
# and the assets folder will not be found. But this has to be build/bin
set_target_properties(openEmpiresTests PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)

target_link_libraries(openEmpiresTests PRIVATE game core GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

target_compile_definitions(openEmpiresTests PRIVATE
    TEST_PYTHON_MODEL_DIR="${CMAKE_SOURCE_DIR}/tests/python"
)
target_compile_definitions(openEmpiresTests PRIVATE
    PYTHON_MODEL_DIR="${CMAKE_SOURCE_DIR}/assets/scripts"
)
# Include directories for the C++ source files
include_directories(${CMAKE_SOURCE_DIR}/src/cpp)
enable_testing()
add_test(NAME openEmpiresTests COMMAND openEmpiresTests)
