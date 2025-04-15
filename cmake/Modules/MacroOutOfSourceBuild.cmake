macro(MacroOutOfSourceBuild)
  if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR
      "🚫 In-source builds are not allowed!\n"
      "✅ Please use the build directory!\n"
    )
  endif()
endmacro()