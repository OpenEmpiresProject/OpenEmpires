add_subdirectory(game)
add_subdirectory(utils)
add_subdirectory(aion)

add_executable(openEmpires main.cpp)

target_link_libraries(openEmpires
    PRIVATE
        game
        utils
        aion
)

target_include_directories(openEmpires
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/game
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${CMAKE_CURRENT_SOURCE_DIR}/aion
)