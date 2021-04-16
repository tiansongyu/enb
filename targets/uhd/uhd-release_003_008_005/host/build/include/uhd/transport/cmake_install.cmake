# Install script for directory: /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/include/uhd/transport

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
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/uhd/transport" TYPE FILE FILES
    "/home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/include/uhd/transport/bounded_buffer.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/include/uhd/transport/bounded_buffer.ipp"
    "/home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/include/uhd/transport/buffer_pool.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/include/uhd/transport/if_addrs.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/include/uhd/transport/udp_constants.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/include/uhd/transport/udp_simple.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/include/uhd/transport/udp_zero_copy.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/include/uhd/transport/tcp_zero_copy.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/include/uhd/transport/usb_control.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/include/uhd/transport/usb_zero_copy.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/include/uhd/transport/usb_device_handle.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/include/uhd/transport/vrt_if_packet.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/include/uhd/transport/zero_copy.hpp"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "headers")

