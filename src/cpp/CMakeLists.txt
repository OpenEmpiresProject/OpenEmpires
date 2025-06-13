add_subdirectory(drs)
add_subdirectory(game)
add_subdirectory(ion)
add_subdirectory(sdl3_gfx)

add_executable(openEmpires main.cpp)

# Without setting this property, the working directory will be build/src/cpp
# and the assets folder will not be found. But this has to be build/bin
set_target_properties(openEmpires PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)

target_link_libraries(openEmpires
    PRIVATE
        drs
        game
        ion
        sdl3_gfx
        microui
)

target_include_directories(openEmpires
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/drs
        ${CMAKE_CURRENT_SOURCE_DIR}/game
        ${CMAKE_CURRENT_SOURCE_DIR}/ion
        ${CMAKE_CURRENT_SOURCE_DIR}/sdl3_gfx
)


# Define the source asset directory
set(ASSET_DIR "${CMAKE_SOURCE_DIR}/assets")
set(OUTPUT_BIN_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}") # Use the existing bin directory

# Create the assets directory in the bin directory if it doesn't exist
file(MAKE_DIRECTORY "${OUTPUT_BIN_DIR}/assets")


# Copy the entire assets directory to the bin directory
add_custom_command(
    TARGET openEmpires
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copying assets..."
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSET_DIR}" "${OUTPUT_BIN_DIR}/assets"
    COMMENT "Copying assets to the binary directory"
    VERBATIM
)