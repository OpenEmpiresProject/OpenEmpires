set(CMAKE_CXX_STANDARD 11)

# Include directories for the C++ source files
include_directories(${CMAKE_SOURCE_DIR}/src/cpp)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Fetch gtest from GitHub and link it to the tests
include(FetchContent)

# Download GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)


# Add the executable for the tests
add_executable(openEmpiresTests test_main.cpp)

# Link the test executable with game libraries and gtest
target_link_libraries(openEmpiresTests PRIVATE game utils gtest_main)

target_include_directories(openEmpiresTests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/cpp/game
        ${CMAKE_SOURCE_DIR}/src/cpp/utils
)

include(GoogleTest)

# Add the C++ tests
add_test(NAME openEmpiresTests COMMAND openEmpiresTests)
