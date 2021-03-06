# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build

# Utility rule file for doxygen_docs.

# Include the progress variables for this target.
include docs/CMakeFiles/doxygen_docs.dir/progress.make

docs/CMakeFiles/doxygen_docs: docs/doxygen

docs/doxygen: ../docs/converters.dox
docs/doxygen: ../docs/build.dox
docs/doxygen: ../docs/multiple.dox
docs/doxygen: ../docs/uhd.dox
docs/doxygen: ../docs/gpsdo.dox
docs/doxygen: ../docs/ni_rio_kernel.dox
docs/doxygen: ../docs/general.dox
docs/doxygen: ../docs/usrp_b100.dox
docs/doxygen: ../docs/coding.dox
docs/doxygen: ../docs/gpio_api.dox
docs/doxygen: ../docs/transport.dox
docs/doxygen: ../docs/images.dox
docs/doxygen: ../docs/identification.dox
docs/doxygen: ../docs/sync.dox
docs/doxygen: ../docs/configuration.dox
docs/doxygen: ../docs/dboards.dox
docs/doxygen: ../docs/devices.dox
docs/doxygen: ../docs/stream.dox
docs/doxygen: ../docs/gpsdo_x3x0.dox
docs/doxygen: ../docs/usrp_e3x0.dox
docs/doxygen: ../docs/usrp2.dox
docs/doxygen: ../docs/usrp_e1x0.dox
docs/doxygen: ../docs/calibration.dox
docs/doxygen: ../docs/c_api.dox
docs/doxygen: ../docs/vrt_chdr.dox
docs/doxygen: ../docs/usrp_x3x0.dox
docs/doxygen: ../docs/usrp1.dox
docs/doxygen: ../docs/install.dox
docs/doxygen: ../docs/gpsdo_b2x0.dox
docs/doxygen: ../docs/usrp_b200.dox
docs/doxygen: ../docs/usrp_x3x0_config.dox
docs/doxygen: ../docs/octoclock.dox
docs/doxygen: ../include/uhd/deprecated.hpp
docs/doxygen: ../include/uhd/convert.hpp
docs/doxygen: ../include/uhd/transport/udp_simple.hpp
docs/doxygen: ../include/uhd/transport/tcp_zero_copy.hpp
docs/doxygen: ../include/uhd/transport/if_addrs.hpp
docs/doxygen: ../include/uhd/transport/chdr.hpp
docs/doxygen: ../include/uhd/transport/udp_constants.hpp
docs/doxygen: ../include/uhd/transport/vrt_if_packet.hpp
docs/doxygen: ../include/uhd/transport/zero_copy.hpp
docs/doxygen: ../include/uhd/transport/usb_zero_copy.hpp
docs/doxygen: ../include/uhd/transport/buffer_pool.hpp
docs/doxygen: ../include/uhd/transport/usb_device_handle.hpp
docs/doxygen: ../include/uhd/transport/usb_control.hpp
docs/doxygen: ../include/uhd/transport/bounded_buffer.hpp
docs/doxygen: ../include/uhd/transport/nirio/rpc/rpc_client.hpp
docs/doxygen: ../include/uhd/transport/nirio/rpc/usrprio_rpc_client.hpp
docs/doxygen: ../include/uhd/transport/nirio/rpc/rpc_common.hpp
docs/doxygen: ../include/uhd/transport/nirio/rpc/usrprio_rpc_common.hpp
docs/doxygen: ../include/uhd/transport/nirio_zero_copy.hpp
docs/doxygen: ../include/uhd/transport/udp_zero_copy.hpp
docs/doxygen: ../include/uhd/property_tree.hpp
docs/doxygen: ../include/uhd/usrp/dboard_id.hpp
docs/doxygen: ../include/uhd/usrp/mboard_eeprom.hpp
docs/doxygen: ../include/uhd/usrp/subdev_spec.hpp
docs/doxygen: ../include/uhd/usrp/dboard_manager.hpp
docs/doxygen: ../include/uhd/usrp/dboard_eeprom.hpp
docs/doxygen: ../include/uhd/usrp/dboard_iface.hpp
docs/doxygen: ../include/uhd/usrp/dboard_base.hpp
docs/doxygen: ../include/uhd/usrp/gps_ctrl.hpp
docs/doxygen: ../include/uhd/usrp/multi_usrp.hpp
docs/doxygen: ../include/uhd/stream.hpp
docs/doxygen: ../include/uhd/image_loader.hpp
docs/doxygen: ../include/uhd/exception.hpp
docs/doxygen: ../include/uhd/device.hpp
docs/doxygen: ../include/uhd/usrp_clock/octoclock_eeprom.hpp
docs/doxygen: ../include/uhd/usrp_clock/multi_usrp_clock.hpp
docs/doxygen: ../include/uhd/utils/platform.hpp
docs/doxygen: ../include/uhd/utils/csv.hpp
docs/doxygen: ../include/uhd/utils/dirty_tracked.hpp
docs/doxygen: ../include/uhd/utils/thread_priority.hpp
docs/doxygen: ../include/uhd/utils/log.hpp
docs/doxygen: ../include/uhd/utils/pimpl.hpp
docs/doxygen: ../include/uhd/utils/assert_has.hpp
docs/doxygen: ../include/uhd/utils/static.hpp
docs/doxygen: ../include/uhd/utils/soft_register.hpp
docs/doxygen: ../include/uhd/utils/msg_task.hpp
docs/doxygen: ../include/uhd/utils/cast.hpp
docs/doxygen: ../include/uhd/utils/safe_call.hpp
docs/doxygen: ../include/uhd/utils/atomic.hpp
docs/doxygen: ../include/uhd/utils/msg.hpp
docs/doxygen: ../include/uhd/utils/paths.hpp
docs/doxygen: ../include/uhd/utils/algorithm.hpp
docs/doxygen: ../include/uhd/utils/tasks.hpp
docs/doxygen: ../include/uhd/utils/safe_main.hpp
docs/doxygen: ../include/uhd/utils/gain_group.hpp
docs/doxygen: ../include/uhd/utils/byteswap.hpp
docs/doxygen: ../include/uhd/utils/math.hpp
docs/doxygen: ../include/uhd/config.hpp
docs/doxygen: ../include/uhd/types/io_type.hpp
docs/doxygen: ../include/uhd/types/sid.hpp
docs/doxygen: ../include/uhd/types/direction.hpp
docs/doxygen: ../include/uhd/types/device_addr.hpp
docs/doxygen: ../include/uhd/types/metadata.hpp
docs/doxygen: ../include/uhd/types/ref_vector.hpp
docs/doxygen: ../include/uhd/types/time_spec.hpp
docs/doxygen: ../include/uhd/types/tune_result.hpp
docs/doxygen: ../include/uhd/types/wb_iface.hpp
docs/doxygen: ../include/uhd/types/serial.hpp
docs/doxygen: ../include/uhd/types/mac_addr.hpp
docs/doxygen: ../include/uhd/types/otw_type.hpp
docs/doxygen: ../include/uhd/types/filters.hpp
docs/doxygen: ../include/uhd/types/ranges.hpp
docs/doxygen: ../include/uhd/types/stream_cmd.hpp
docs/doxygen: ../include/uhd/types/dict.hpp
docs/doxygen: ../include/uhd/types/endianness.hpp
docs/doxygen: ../include/uhd/types/sensors.hpp
docs/doxygen: ../include/uhd/types/clock_config.hpp
docs/doxygen: ../include/uhd/types/byte_vector.hpp
docs/doxygen: ../include/uhd/types/tune_request.hpp
docs/doxygen: ../include/uhd/transport/nirio/niriok_proxy_impl_v2.h
docs/doxygen: ../include/uhd/transport/nirio/nifpga_lvbitx.h
docs/doxygen: ../include/uhd/transport/nirio/nirio_err_template.h
docs/doxygen: ../include/uhd/transport/nirio/niriok_proxy_impl_v1.h
docs/doxygen: ../include/uhd/transport/nirio/niriok_proxy.h
docs/doxygen: ../include/uhd/transport/nirio/nirio_resource_manager.h
docs/doxygen: ../include/uhd/transport/nirio/niusrprio_session.h
docs/doxygen: ../include/uhd/transport/nirio/status.h
docs/doxygen: ../include/uhd/transport/nirio/nirio_driver_iface.h
docs/doxygen: ../include/uhd/transport/nirio/nirio_fifo.h
docs/doxygen: ../include/uhd/transport/nirio/nirio_quirks.h
docs/doxygen: ../include/uhd/usrp/subdev_spec.h
docs/doxygen: ../include/uhd/usrp/mboard_eeprom.h
docs/doxygen: ../include/uhd/usrp/usrp.h
docs/doxygen: ../include/uhd/usrp/dboard_eeprom.h
docs/doxygen: ../include/uhd/config.h
docs/doxygen: ../include/uhd/error.h
docs/doxygen: ../include/uhd/usrp_clock/usrp_clock.h
docs/doxygen: ../include/uhd/utils/thread_priority.h
docs/doxygen: ../include/uhd/types/usrp_info.h
docs/doxygen: ../include/uhd/types/sensors.h
docs/doxygen: ../include/uhd/types/string_vector.h
docs/doxygen: ../include/uhd/types/tune_request.h
docs/doxygen: ../include/uhd/types/metadata.h
docs/doxygen: ../include/uhd/types/ranges.h
docs/doxygen: ../include/uhd/types/tune_result.h
docs/doxygen: ../include/uhd.h
	$(CMAKE_COMMAND) -E cmake_progress_report /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold "Generating documentation with doxygen"
	cd /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/docs && /usr/bin/doxygen /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/docs/Doxyfile

doxygen_docs: docs/CMakeFiles/doxygen_docs
doxygen_docs: docs/doxygen
doxygen_docs: docs/CMakeFiles/doxygen_docs.dir/build.make
.PHONY : doxygen_docs

# Rule to build all files generated by this target.
docs/CMakeFiles/doxygen_docs.dir/build: doxygen_docs
.PHONY : docs/CMakeFiles/doxygen_docs.dir/build

docs/CMakeFiles/doxygen_docs.dir/clean:
	cd /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/docs && $(CMAKE_COMMAND) -P CMakeFiles/doxygen_docs.dir/cmake_clean.cmake
.PHONY : docs/CMakeFiles/doxygen_docs.dir/clean

docs/CMakeFiles/doxygen_docs.dir/depend:
	cd /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/docs /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/docs /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/docs/CMakeFiles/doxygen_docs.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : docs/CMakeFiles/doxygen_docs.dir/depend

