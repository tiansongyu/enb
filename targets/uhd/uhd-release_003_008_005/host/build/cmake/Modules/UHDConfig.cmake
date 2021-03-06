#
# Copyright 2014 Ettus Research LLC
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
########################################################################
#
# Find the header <uhd/config.hpp> and library "libuhd" for the USRP
# Hardware Driver.  Priorty for prefix search is:
# 1) ENV(UHD_DIR)
# 2) pkg-config results, if available;
# 3) CMAKE_INSTALL_PREFIX
# 4) /usr/local/
# 5) /usr/
#
# Version info is handled by UHDConfigVersion.cmake only; not here.
#
########################################################################

# set that this file was found, for use in GNU Radio's FindUHD.cmake.
# Have to use the ENV, since this file might not allow CACHE changes.

set(ENV{UHD_CONFIG_USED} TRUE)

# set default values

SET(UHD_FOUND TRUE)
SET(UHD_INCLUDE_HINTS)
SET(UHD_LIBDIR_HINTS)
SET(UHD_DIR $ENV{UHD_DIR})

IF(UHD_DIR)
    LIST(APPEND UHD_INCLUDE_HINTS ${UHD_DIR}/include)
    LIST(APPEND UHD_LIBDIR_HINTS ${UHD_DIR}/lib)
ENDIF()

INCLUDE(FindPkgConfig)
IF(PKG_CONFIG_FOUND)
  IF(NOT ${CMAKE_VERSION} VERSION_LESS "2.8.0")
    SET(UHD_QUIET "QUIET")
  ENDIF()
  PKG_CHECK_MODULES(PC_UHD ${UHD_QUIET} uhd)
  IF(PC_UHD_FOUND)
    LIST(APPEND UHD_INCLUDE_HINTS ${PC_UHD_INCLUDEDIR})
    LIST(APPEND UHD_LIBDIR_HINTS ${PC_UHD_LIBDIR})
  ENDIF()
ENDIF()

LIST(APPEND UHD_INCLUDE_HINTS ${CMAKE_INSTALL_PREFIX}/include)
LIST(APPEND UHD_LIBDIR_HINTS ${CMAKE_INSTALL_PREFIX}/lib)


# Search for static libs if so required
IF( UHD_USE_STATIC_LIBS )
    SET( _UHD_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
    IF(WIN32)
        SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
    ELSE()
        SET(CMAKE_FIND_LIBRARY_SUFFIXES .a )
    ENDIF()
    # This is set during build of UHD to match the installed version:
    SET(UHD_STATIC_LIB_DEPS "dl;pthread;boost_date_time;boost_filesystem;boost_program_options;boost_regex;boost_system;boost_thread;boost_serialization;usb-1.0")
ENDIF()

# Verify that <uhd/config.hpp> and libuhd are available, and, if a
# version is provided, that UHD meets the version requirements -- no
# matter what pkg-config might think.

FIND_PATH(
    UHD_INCLUDE_DIRS
    NAMES uhd/config.hpp
    HINTS ${UHD_INCLUDE_HINTS}
    PATHS /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    UHD_LIBRARIES
    NAMES uhd
    HINTS ${UHD_LIBDIR_HINTS}
    PATHS /usr/local/lib
          /usr/lib
)

# Set up linker flags for static linking:
IF(UHD_USE_STATIC_LIBS)
    IF(WIN32)
        MESSAGE(FATAL_ERROR "Static linking not available on Windows")
    ELSE(WIN32)
        # This works for gcc and Clang:
        SET(UHD_STATIC_LIB_LINK_FLAG "-Wl,-whole-archive ${UHD_LIBRARIES} -Wl,-no-whole-archive")
    ENDIF(WIN32)
    # Restore the library suffixes, if we changed them:
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ${_UHD_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES})
ENDIF(UHD_USE_STATIC_LIBS)

IF(UHD_LIBRARIES AND UHD_INCLUDE_DIRS)

  INCLUDE(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(UHD DEFAULT_MSG UHD_LIBRARIES UHD_INCLUDE_DIRS)
  MARK_AS_ADVANCED(UHD_LIBRARIES UHD_INCLUDE_DIRS)

ELSEIF(UHD_FIND_REQUIRED)

  MESSAGE(FATAL_ERROR "UHD is required, but was not found.")

ENDIF()
