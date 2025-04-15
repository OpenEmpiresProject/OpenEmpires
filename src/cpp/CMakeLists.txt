add_subdirectory(game)
add_subdirectory(utils)

add_executable(openEmpires main.cpp)

target_link_libraries(openEmpires
    PRIVATE
        game
        utils
)

target_include_directories(openEmpires
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/game
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
)