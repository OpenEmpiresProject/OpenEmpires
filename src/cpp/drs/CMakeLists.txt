find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)

# Automatically gather .cpp files (headers are for IDEs)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    *.cpp
    *.h *.hpp
)

add_library(drs STATIC ${SOURCES})

target_include_directories(drs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(drs 
    PUBLIC 
        SDL3::SDL3
        $<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image-shared>,SDL3_image::SDL3_image-shared,SDL3_image::SDL3_image-static>
)
