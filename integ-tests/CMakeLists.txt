
find_package(GTest CONFIG REQUIRED)

file(GLOB_RECURSE INTEG_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_executable(openEmpiresIntegTests ${INTEG_TEST_SOURCES})
target_link_libraries(openEmpiresIntegTests PRIVATE game ion GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

# Include directories for the C++ source files
include_directories(${CMAKE_SOURCE_DIR}/src/cpp)
enable_testing()
add_test(NAME openEmpiresIntegTests COMMAND openEmpiresIntegTests)
