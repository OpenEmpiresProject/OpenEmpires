find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)

include(FetchContent)

FetchContent_Declare(
  readerwriterqueue
  GIT_REPOSITORY    https://github.com/cameron314/readerwriterqueue
  GIT_TAG           master
)
FetchContent_MakeAvailable(readerwriterqueue)

FetchContent_Declare(
  microui
  GIT_REPOSITORY    https://github.com/rxi/microui
  GIT_TAG           master
)
FetchContent_GetProperties(microui)
if(NOT microui_POPULATED)
  FetchContent_Populate(microui)

  # Check if microui.c exists
  if(EXISTS "${microui_SOURCE_DIR}/src/microui.c")
    message(STATUS "microui successfully downloaded to: ${microui_SOURCE_DIR}")
  else()
    message(FATAL_ERROR "microui download failed or does not contain microui.c")
  endif()

  add_library(microui STATIC
    ${microui_SOURCE_DIR}/src/microui.c
  )

  # set_target_properties(microui PROPERTIES LINKER_LANGUAGE C)

  target_include_directories(microui PUBLIC
    ${microui_SOURCE_DIR}/src
  )
  target_compile_options(microui PUBLIC -std=c99)

  set_target_properties(microui PROPERTIES
    LINKER_LANGUAGE C
    C_STANDARD 99
)
endif()

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
    microui
)