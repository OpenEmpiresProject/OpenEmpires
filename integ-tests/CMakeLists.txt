
find_package(GTest CONFIG REQUIRED)

file(GLOB_RECURSE INTEG_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_executable(openEmpiresIntegTests ${INTEG_TEST_SOURCES})

# Without setting this property, the working directory will be build/src/cpp
# and the assets folder will not be found. But this has to be build/bin
set_target_properties(openEmpiresIntegTests PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)

target_link_libraries(openEmpiresIntegTests PRIVATE game ion GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

# Include directories for the C++ source files
include_directories(${CMAKE_SOURCE_DIR}/src/cpp)
enable_testing()
add_test(NAME openEmpiresIntegTests COMMAND openEmpiresIntegTests)
