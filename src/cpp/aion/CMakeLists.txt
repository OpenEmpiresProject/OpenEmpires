find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)

# TODO: None of the following SDL3_gfx package imports work. Therefore, we have
# our own copy of SDL3_gfx in the src/cpp/sdl3_gfx directory. We can remove this
# later when we have a working solution for SDL3_gfx.

# Method1: Find SDL3_gfx using CMake's find_package: DOESN"T WORK
# Alias SDL3::SDL3 to SDL3_LIB (before adding your library)
# add_library(SDL3_LIB INTERFACE IMPORTED)
# target_link_libraries(SDL3_LIB INTERFACE SDL3::SDL3)

# list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
# include(FindSDL3_gfx)

# find_package(SDL3_gfx REQUIRED)

include(FetchContent)

FetchContent_Declare(
  readerwriterqueue
  GIT_REPOSITORY    https://github.com/cameron314/readerwriterqueue
  GIT_TAG           master
)

# Method2: Find SDL3_gfx by fetching: DOESN"T WORK
# FetchContent_Declare(
#   SDL3
#   GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
#   GIT_TAG main
# )
# FetchContent_Declare(
#     SDL3_gfx
#     GIT_REPOSITORY https://github.com/sabdul-khabir/SDL3_gfx
#     GIT_TAG master # Or a specific commit/tag if you prefer
#     CMAKE_ARGS -DSDL3_INCLUDE_DIR="${SDL3_INCLUDE_DIR}" 
#     # By default, FetchContent creates a subdirectory named "SDL3_gfx"
# )
# FetchContent_MakeAvailable(SDL3)
# FetchContent_MakeAvailable(SDL3_gfx)

FetchContent_MakeAvailable(readerwriterqueue)

# Automatically gather .cpp files (headers are for IDEs)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    *.cpp
    *.h *.hpp
)

add_library(aion STATIC ${SOURCES})

target_include_directories(aion PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(aion PUBLIC 
    readerwriterqueue
    EnTT::EnTT
    SDL3::SDL3
    $<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image-shared>,SDL3_image::SDL3_image-shared,SDL3_image::SDL3_image-static>
    sdl3_gfx
)
