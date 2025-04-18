# Automatically gather .cpp files (headers are for IDEs)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    *.cpp
    *.h *.hpp
)

add_library(spice STATIC ${SOURCES})

target_include_directories(spice PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(spice PUBLIC utils)
