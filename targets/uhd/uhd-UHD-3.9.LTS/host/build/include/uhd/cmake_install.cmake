# Install script for directory: /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Release")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "1")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "headers")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/uhd" TYPE FILE FILES
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/config.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/convert.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/deprecated.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/device.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/device_deprecated.ipp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/exception.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/property_tree.ipp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/property_tree.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/stream.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/include/uhd/version.hpp"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "headers")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "headers")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/uhd" TYPE FILE FILES
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/config.h"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/error.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "headers")

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/include/uhd/transport/cmake_install.cmake")
  INCLUDE("/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/include/uhd/types/cmake_install.cmake")
  INCLUDE("/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/include/uhd/usrp/cmake_install.cmake")
  INCLUDE("/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/include/uhd/usrp_clock/cmake_install.cmake")
  INCLUDE("/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/include/uhd/utils/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

