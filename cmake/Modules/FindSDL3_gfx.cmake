#TODO : This doesn't work.

#[=======================================================================[.rst:
FindSDL3_gfx
--------------

Locate SDL3_gfx library

This module defines the following 'IMPORTED' target:

::

  SDL3::GFX
    The SDL3_gfx library, if found.
    Have SDL3::Core as a link dependency.



This module will set the following variables in your project:

::

  SDL3_GFX_LIBRARIES, the name of the library to link against
  SDL3_GFX_INCLUDE_DIRS, where to find the headers
  SDL3_GFX_FOUND, if false, do not try to link against
  SDL3_GFX_VERSION_STRING - human-readable string containing the
                              version of SDL3_gfx



This module responds to the following cache variables:

::

  SDL3_GFX_PATH
    Set a custom SDL3_gfx Library path (default: empty)

  SDL3_GFX_NO_DEFAULT_PATH
    Disable search SDL3_gfx Library in default path.
      If SDL3_GFX_PATH (default: ON)
      Else (default: OFF)

  SDL3_GFX_INCLUDE_DIR
    SDL3_gfx headers path.

  SDL3_GFX_LIBRARY
    SDL3_gfx Library (.dll, .so, .a, etc) path.


Additional Note: If you see an empty SDL3_GFX_LIBRARY in your project
configuration, it means CMake did not find your SDL3_gfx library
(SDL3_gfx.dll, libsdl2_gfx.so, etc). Set SDL3_GFX_LIBRARY to point
to your SDL3_gfx library, and  configure again. This value is used to
generate the final SDL3_GFX_LIBRARIES variable and the SDL3::GFX target,
but when this value is unset, SDL3_GFX_LIBRARIES and SDL3::GFX does not
get created.


$SDL3GFXDIR is an environment variable that would correspond to the
./configure --prefix=$SDL3GFXDIR used in building SDL3_gfx.

$SDL3DIR is an environment variable that would correspond to the
./configure --prefix=$SDL3DIR used in building SDL3.



Created by Amine Ben Hassouna:
  Adapt FindSDL_image.cmake to SDL3_gfx (FindSDL3_gfx.cmake).
  Add cache variables for more flexibility:
    SDL3_GFX_PATH, SDL3_GFX_NO_DEFAULT_PATH (for details, see doc above).
  Add SDL3 as a required dependency.
  Modernize the FindSDL3_gfx.cmake module by creating a specific target:
    SDL3::GFX (for details, see doc above).

Original FindSDL_image.cmake module:
  Created by Eric Wing.  This was influenced by the FindSDL.cmake
  module, but with modifications to recognize OS X frameworks and
  additional Unix paths (FreeBSD, etc).
#]=======================================================================]

message(WARNING "Executing FindSDL3_gfx.cmake module")

# SDL3 Library required
find_package(SDL3 QUIET)
if(NOT SDL3_FOUND)
  set(SDL3_GFX_SDL3_NOT_FOUND "Could NOT find SDL3 (SDL3 is required by SDL3_gfx).")
  if(SDL3_gfx_FIND_REQUIRED)
    message(FATAL_ERROR ${SDL3_GFX_SDL3_NOT_FOUND})
  else()
      if(NOT SDL3_gfx_FIND_QUIETLY)
        message(STATUS ${SDL3_GFX_SDL3_NOT_FOUND})
      endif()
    return()
  endif()
  unset(SDL3_GFX_SDL3_NOT_FOUND)
endif()

message(WARNING "Executing FindSDL3_gfx.cmake module1")

# Define options for searching SDL3_gfx Library in a custom path

set(SDL3_GFX_PATH "" CACHE STRING "Custom SDL3_gfx Library path")

set(_SDL3_GFX_NO_DEFAULT_PATH OFF)
if(SDL3_GFX_PATH)
  set(_SDL3_GFX_NO_DEFAULT_PATH ON)
endif()

set(SDL3_GFX_NO_DEFAULT_PATH ${_SDL3_GFX_NO_DEFAULT_PATH}
    CACHE BOOL "Disable search SDL3_gfx Library in default path")
unset(_SDL3_GFX_NO_DEFAULT_PATH)

set(SDL3_GFX_NO_DEFAULT_PATH_CMD)
if(SDL3_GFX_NO_DEFAULT_PATH)
  set(SDL3_GFX_NO_DEFAULT_PATH_CMD NO_DEFAULT_PATH)
endif()

message(WARNING "Executing FindSDL3_gfx.cmake module2")

# Search for the SDL3_gfx include directory
find_path(SDL3_GFX_INCLUDE_DIR SDL3_gfxPrimitives.h
  HINTS
    ENV SDL3GFXDIR
    ENV SDL3DIR
    ${SDL3_GFX_NO_DEFAULT_PATH_CMD}
  PATH_SUFFIXES SDL3
                # path suffixes to search inside ENV{SDL3DIR}
                # and ENV{SDL3GFXDIR}
                include/SDL3 include
  PATHS ${SDL3_GFX_PATH}
  DOC "Where the SDL3_gfx headers can be found"
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(VC_LIB_PATH_SUFFIX lib/x64)
else()
  set(VC_LIB_PATH_SUFFIX lib/x86)
endif()

message(WARNING "Executing FindSDL3_gfx.cmake module3")

# Search for the SDL3_gfx library
find_library(SDL3_GFX_LIBRARY
  NAMES SDL3_gfx
  HINTS
    ENV SDL3GFXDIR
    ENV SDL3DIR
    ${SDL3_GFX_NO_DEFAULT_PATH_CMD}
  PATH_SUFFIXES lib ${VC_LIB_PATH_SUFFIX}
  PATHS ${SDL3_GFX_PATH}
  DOC "Where the SDL3_gfx Library can be found"
)

# Read SDL3_gfx version
if(SDL3_GFX_INCLUDE_DIR AND EXISTS "${SDL3_GFX_INCLUDE_DIR}/SDL3_gfxPrimitives.h")
  file(STRINGS "${SDL3_GFX_INCLUDE_DIR}/SDL3_gfxPrimitives.h" SDL3_GFX_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL3_GFXPRIMITIVES_MAJOR[ \t]+[0-9]+$")
  file(STRINGS "${SDL3_GFX_INCLUDE_DIR}/SDL3_gfxPrimitives.h" SDL3_GFX_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL3_GFXPRIMITIVES_MINOR[ \t]+[0-9]+$")
  file(STRINGS "${SDL3_GFX_INCLUDE_DIR}/SDL3_gfxPrimitives.h" SDL3_GFX_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL3_GFXPRIMITIVES_MICRO[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL3_GFXPRIMITIVES_MAJOR[ \t]+([0-9]+)$" "\\1" SDL3_GFX_VERSION_MAJOR "${SDL3_GFX_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL3_GFXPRIMITIVES_MINOR[ \t]+([0-9]+)$" "\\1" SDL3_GFX_VERSION_MINOR "${SDL3_GFX_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL3_GFXPRIMITIVES_MICRO[ \t]+([0-9]+)$" "\\1" SDL3_GFX_VERSION_PATCH "${SDL3_GFX_VERSION_PATCH_LINE}")
  set(SDL3_GFX_VERSION_STRING ${SDL3_GFX_VERSION_MAJOR}.${SDL3_GFX_VERSION_MINOR}.${SDL3_GFX_VERSION_PATCH})
  unset(SDL3_GFX_VERSION_MAJOR_LINE)
  unset(SDL3_GFX_VERSION_MINOR_LINE)
  unset(SDL3_GFX_VERSION_PATCH_LINE)
  unset(SDL3_GFX_VERSION_MAJOR)
  unset(SDL3_GFX_VERSION_MINOR)
  unset(SDL3_GFX_VERSION_PATCH)
endif()

message(WARNING "Executing FindSDL3_gfx.cmake module4")


set(SDL3_GFX_LIBRARIES ${SDL3_GFX_LIBRARY})
set(SDL3_GFX_INCLUDE_DIRS ${SDL3_GFX_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL3_gfx
                                  REQUIRED_VARS SDL3_GFX_LIBRARIES SDL3_GFX_INCLUDE_DIRS
                                  VERSION_VAR SDL3_GFX_VERSION_STRING)


mark_as_advanced(SDL3_GFX_PATH
                 SDL3_GFX_NO_DEFAULT_PATH
                 SDL3_GFX_LIBRARY
                 SDL3_GFX_INCLUDE_DIR)


if(SDL3_GFX_FOUND)

  # SDL3::GFX target
  if(SDL3_GFX_LIBRARY AND NOT TARGET SDL3::GFX)
    add_library(SDL3::GFX UNKNOWN IMPORTED)
    set_target_properties(SDL3::GFX PROPERTIES
                          IMPORTED_LOCATION "${SDL3_GFX_LIBRARY}"
                          INTERFACE_INCLUDE_DIRECTORIES "${SDL3_GFX_INCLUDE_DIR}"
                          INTERFACE_LINK_LIBRARIES SDL3::Core)
  endif()
endif()

message(WARNING "Executing FindSDL3_gfx.cmake module5")
