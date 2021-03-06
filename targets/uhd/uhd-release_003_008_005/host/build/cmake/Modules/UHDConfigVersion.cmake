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
# When "find_package" is provided with UHD and a version, this file is
# called to try to determine if the requested version matches that
# provided by this UHD install.  All version checking is done herein.
########################################################################

# set that this file was found, for use in GNU Radio's FindUHD.cmake.
# Have to use the ENV, since this file might not allow CACHE changes.

set(ENV{UHD_CONFIG_VERSION_USED} TRUE)

# version values as set in cmake/Modules/UHDVersion.cmake, placed
# statically in here to avoid using Python all over again.

SET(MAJOR_VERSION 3)
SET(MINOR_VERSION 8)
SET(PATCH_VERSION 5)
SET(DEVEL_VERSION FALSE)

SET(PACKAGE_VERSION 3.8.5)
SET(ENV{UHD_PACKAGE_VERSION} ${PACKAGE_VERSION})

# There is a bug in CMake whereby calling "find_package(FOO)" within
# "find_package(FOO)" results in the version being checked in the
# second version no matter if it was set.  To get around this, check
# "PACKAGE_FIND_VERSION" and if empty set return variables to TRUE to
# make CMake happy.  Not the best solution, but it does the trick.

IF(NOT PACKAGE_FIND_VERSION)
  SET(PACKAGE_VERSION_COMPATIBLE TRUE)
  SET(PACKAGE_VERSION_EXACT TRUE)
  RETURN()
ENDIF(NOT PACKAGE_FIND_VERSION)

# Development branches of UHD don't have a patch version. This is a hack
# to add a fake patch version that should be higher than anything the user
# requests.
IF(DEVEL_VERSION)
  SET(PACKAGE_VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.999")
ENDIF(DEVEL_VERSION)

# assume incorrect versioning by default
SET(PACKAGE_VERSION_COMPATIBLE FALSE)
SET(PACKAGE_VERSION_EXACT FALSE)

# do not use ABI for now
SET(UHD_USE_ABI FALSE)

# leave the ABI checking in, for now, just in case it is wanted in the
# future.  This code works nicely to find the ABI compatibility
# version from <uhd/version.hpp>.
IF(UHD_USE_ABI)

  # find ABI compatible version from <uhd/version.hpp>

  SET(UHD_INCLUDE_HINTS)
  SET(UHD_DIR $ENV{UHD_DIR})

  IF(UHD_DIR)
    LIST(APPEND UHD_INCLUDE_HINTS ${UHD_DIR}/include)
  ENDIF()

  INCLUDE(FindPkgConfig)
  IF(PKG_CONFIG_FOUND)
    IF(NOT ${CMAKE_VERSION} VERSION_LESS "2.8.0")
      SET(UHD_QUIET "QUIET")
    ENDIF()
    IF(PACKAGE_VERSION_EXACT)
      PKG_CHECK_MODULES(PC_UHD ${UHD_QUIET} uhd=${UHD_FIND_VERSION})
    ELSE()
      PKG_CHECK_MODULES(PC_UHD ${UHD_QUIET} uhd>=${UHD_FIND_VERSION})
    ENDIF()
    IF(PC_UHD_FOUND)
      LIST(APPEND UHD_INCLUDE_HINTS ${PC_UHD_INCLUDEDIR})
    ENDIF()
  ENDIF()

  LIST(APPEND UHD_INCLUDE_HINTS ${CMAKE_INSTALL_PREFIX}/include)

  # Verify that <uhd/config.hpp> and libuhd are available, and, if a
  # version is provided, that UHD meets the version requirements -- no
  # matter what pkg-config might think.

  FIND_PATH(
    UHD_INCLUDE_DIR
    NAMES uhd/version.hpp
    HINTS ${UHD_INCLUDE_HINTS}
    PATHS /usr/local/include
          /usr/include
  )

  IF(UHD_INCLUDE_DIR)

    # extract the UHD API version from the installed header

    FILE(STRINGS "${UHD_INCLUDE_DIR}/uhd/version.hpp"
      UHD_STRING_VERSION REGEX "UHD_VERSION_ABI_STRING")
    STRING(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+"
      UHD_ABI_VERSION_CONCISE ${UHD_STRING_VERSION})

    # convert UHD_FIND_VERSION into concise #.#.# format for comparison

    STRING(REGEX REPLACE "([^\\.]*)\\.([^\\.]*)\\.([^\\.]*)"
      "\\1.\\2.\\3" UHD_ABI_VERSION_TMP ${UHD_ABI_VERSION_CONCISE})

    STRING(REPLACE "0" "" UHD_ABI_MAJOR ${CMAKE_MATCH_1})
    STRING(REPLACE "0" "" UHD_ABI_MINOR ${CMAKE_MATCH_2})
    STRING(REPLACE "0" "" UHD_ABI_PATCH ${CMAKE_MATCH_3})

    # fix the case where the version number is "000"

    IF(NOT UHD_ABI_MAJOR)
      SET(UHD_ABI_MAJOR "0")
    ENDIF()
    IF(NOT UHD_ABI_MINOR)
      SET(UHD_ABI_MINOR "0")
    ENDIF()
    IF(NOT UHD_ABI_PATCH)
      SET(UHD_ABI_PATCH "0")
    ENDIF()

    SET(UHD_ABI_VERSION_CONCISE ${UHD_ABI_MAJOR}.${UHD_ABI_MINOR}.${UHD_ABI_PATCH})

  ELSE(UHD_INCLUDE_DIR)

    # no header found ... not a good sign!  Assume ABI version is the
    # same as that known internally here.  Let UHDConfig.cmake fail if
    # it cannot find <uhd/config.hpp> or "libuhd" ...

    SET(UHD_ABI_VERSION_CONCISE ${PACKAGE_VERSION})

  ENDIF(UHD_INCLUDE_DIR)

  # check for ABI compatibility, both:
  #   ACTUAL VERSION >= DESIRED VERSION >= ABI VERSION

  IF(NOT ${PACKAGE_FIND_VERSION} VERSION_LESS ${UHD_ABI_VERSION_CONCISE} AND
     NOT ${PACKAGE_FIND_VERSION} VERSION_GREATER ${PACKAGE_VERSION})
    SET(PACKAGE_VERSION_COMPATIBLE TRUE)
  ENDIF()

ELSE(UHD_USE_ABI)

  # use API only, and assume compatible of requested <= actual
  # which is the same as "not >"

  IF(NOT ${PACKAGE_FIND_VERSION} VERSION_GREATER ${PACKAGE_VERSION})
    SET(PACKAGE_VERSION_COMPATIBLE TRUE)
  ENDIF()

ENDIF(UHD_USE_ABI)

# check for exact version

IF(${PACKAGE_FIND_VERSION} VERSION_EQUAL ${PACKAGE_VERSION})
  SET(PACKAGE_VERSION_EXACT TRUE)
ENDIF()

# Undo our patch-version-number hack
SET(PACKAGE_VERSION 3.8.5)
