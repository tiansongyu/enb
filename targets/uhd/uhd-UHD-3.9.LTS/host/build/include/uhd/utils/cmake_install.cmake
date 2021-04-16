# Install script for directory: /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils

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
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/uhd/utils" TYPE FILE FILES
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/algorithm.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/assert_has.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/assert_has.ipp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/atomic.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/byteswap.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/byteswap.ipp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/cast.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/csv.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/fp_compare_delta.ipp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/fp_compare_epsilon.ipp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/gain_group.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/log.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/math.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/msg.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/msg_task.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/paths.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/pimpl.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/platform.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/safe_call.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/safe_main.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/static.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/tasks.hpp"
    "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/thread_priority.hpp"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "headers")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "headers")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/uhd/utils" TYPE FILE FILES "/home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/include/uhd/utils/thread_priority.h")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "headers")

