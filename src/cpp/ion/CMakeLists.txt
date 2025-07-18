find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

include(FetchContent)

FetchContent_Declare(
  readerwriterqueue
  GIT_REPOSITORY    https://github.com/cameron314/readerwriterqueue
  GIT_TAG           master
)
FetchContent_MakeAvailable(readerwriterqueue)

# Automatically gather .cpp files (headers are for IDEs)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    *.cpp
    *.h *.hpp
)

add_library(ion STATIC ${SOURCES})

target_include_directories(ion PUBLIC ${GENERATED_DIR})
target_include_directories(ion PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ion PUBLIC 
    readerwriterqueue
    EnTT::EnTT
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    $<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image-shared>,SDL3_image::SDL3_image-shared,SDL3_image::SDL3_image-static>
    sdl3_gfx
    pybind11::embed
)