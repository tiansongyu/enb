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
CMAKE_SOURCE_DIR = /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build

# Include any dependencies generated for this target.
include utils/CMakeFiles/uhd_find_devices.dir/depend.make

# Include the progress variables for this target.
include utils/CMakeFiles/uhd_find_devices.dir/progress.make

# Include the compile flags for this target's objects.
include utils/CMakeFiles/uhd_find_devices.dir/flags.make

utils/CMakeFiles/uhd_find_devices.dir/uhd_find_devices.cpp.o: utils/CMakeFiles/uhd_find_devices.dir/flags.make
utils/CMakeFiles/uhd_find_devices.dir/uhd_find_devices.cpp.o: ../utils/uhd_find_devices.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object utils/CMakeFiles/uhd_find_devices.dir/uhd_find_devices.cpp.o"
	cd /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/utils && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/uhd_find_devices.dir/uhd_find_devices.cpp.o -c /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/utils/uhd_find_devices.cpp

utils/CMakeFiles/uhd_find_devices.dir/uhd_find_devices.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/uhd_find_devices.dir/uhd_find_devices.cpp.i"
	cd /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/utils && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/utils/uhd_find_devices.cpp > CMakeFiles/uhd_find_devices.dir/uhd_find_devices.cpp.i

utils/CMakeFiles/uhd_find_devices.dir/uhd_find_devices.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/uhd_find_devices.dir/uhd_find_devices.cpp.s"
	cd /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/utils && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/utils/uhd_find_devices.cpp -o CMakeFiles/uhd_find_devices.dir/uhd_find_devices.cpp.s

utils/CMakeFiles/uhd_find_devices.dir/uhd_find_devices.cpp.o.requires:
.PHONY : utils/CMakeFiles/uhd_find_devices.dir/uhd_find_devices.cpp.o.requires

utils/CMakeFiles/uhd_find_devices.dir/uhd_find_devices.cpp.o.provides: utils/CMakeFiles/uhd_find_devices.dir/uhd_find_devices.cpp.o.requires
	$(MAKE) -f utils/CMakeFiles/uhd_find_devices.dir/build.make utils/CMakeFiles/uhd_find_devices.dir/uhd_find_devices.cpp.o.provides.build
.PHONY : utils/CMakeFiles/uhd_find_devices.dir/uhd_find_devices.cpp.o.provides

utils/CMakeFiles/uhd_find_devices.dir/uhd_find_devices.cpp.o.provides.build: utils/CMakeFiles/uhd_find_devices.dir/uhd_find_devices.cpp.o

# Object files for target uhd_find_devices
uhd_find_devices_OBJECTS = \
"CMakeFiles/uhd_find_devices.dir/uhd_find_devices.cpp.o"

# External object files for target uhd_find_devices
uhd_find_devices_EXTERNAL_OBJECTS =

utils/uhd_find_devices: utils/CMakeFiles/uhd_find_devices.dir/uhd_find_devices.cpp.o
utils/uhd_find_devices: utils/CMakeFiles/uhd_find_devices.dir/build.make
utils/uhd_find_devices: lib/libuhd.so.003.008
utils/uhd_find_devices: /usr/lib/x86_64-linux-gnu/libboost_date_time.so
utils/uhd_find_devices: /usr/lib/x86_64-linux-gnu/libboost_filesystem.so
utils/uhd_find_devices: /usr/lib/x86_64-linux-gnu/libboost_program_options.so
utils/uhd_find_devices: /usr/lib/x86_64-linux-gnu/libboost_regex.so
utils/uhd_find_devices: /usr/lib/x86_64-linux-gnu/libboost_system.so
utils/uhd_find_devices: /usr/lib/x86_64-linux-gnu/libboost_thread.so
utils/uhd_find_devices: /usr/lib/x86_64-linux-gnu/libboost_unit_test_framework.so
utils/uhd_find_devices: /usr/lib/x86_64-linux-gnu/libboost_serialization.so
utils/uhd_find_devices: /usr/lib/x86_64-linux-gnu/libpthread.so
utils/uhd_find_devices: /usr/lib/x86_64-linux-gnu/libusb-1.0.so
utils/uhd_find_devices: utils/CMakeFiles/uhd_find_devices.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable uhd_find_devices"
	cd /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/utils && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/uhd_find_devices.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
utils/CMakeFiles/uhd_find_devices.dir/build: utils/uhd_find_devices
.PHONY : utils/CMakeFiles/uhd_find_devices.dir/build

utils/CMakeFiles/uhd_find_devices.dir/requires: utils/CMakeFiles/uhd_find_devices.dir/uhd_find_devices.cpp.o.requires
.PHONY : utils/CMakeFiles/uhd_find_devices.dir/requires

utils/CMakeFiles/uhd_find_devices.dir/clean:
	cd /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/utils && $(CMAKE_COMMAND) -P CMakeFiles/uhd_find_devices.dir/cmake_clean.cmake
.PHONY : utils/CMakeFiles/uhd_find_devices.dir/clean

utils/CMakeFiles/uhd_find_devices.dir/depend:
	cd /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/utils /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/utils /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/utils/CMakeFiles/uhd_find_devices.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : utils/CMakeFiles/uhd_find_devices.dir/depend
