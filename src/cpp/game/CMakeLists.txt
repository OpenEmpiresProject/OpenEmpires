add_library(game STATIC
    GameEngine.cpp
    GameEngine.h
)

target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(game PUBLIC utils)
