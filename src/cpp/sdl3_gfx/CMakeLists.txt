find_package(spdlog CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)

# Automatically gather .cpp files (headers are for IDEs)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    *.cpp
    *.h *.hpp
)

add_library(sdl3_gfx STATIC ${SOURCES})

target_include_directories(sdl3_gfx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(sdl3_gfx 
    PUBLIC 
    spdlog::spdlog
    DbgHelp
    SDL3::SDL3
)