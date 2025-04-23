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