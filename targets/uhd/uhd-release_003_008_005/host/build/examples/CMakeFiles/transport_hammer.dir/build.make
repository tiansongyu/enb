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
include examples/CMakeFiles/transport_hammer.dir/depend.make

# Include the progress variables for this target.
include examples/CMakeFiles/transport_hammer.dir/progress.make

# Include the compile flags for this target's objects.
include examples/CMakeFiles/transport_hammer.dir/flags.make

examples/CMakeFiles/transport_hammer.dir/transport_hammer.cpp.o: examples/CMakeFiles/transport_hammer.dir/flags.make
examples/CMakeFiles/transport_hammer.dir/transport_hammer.cpp.o: ../examples/transport_hammer.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object examples/CMakeFiles/transport_hammer.dir/transport_hammer.cpp.o"
	cd /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/examples && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/transport_hammer.dir/transport_hammer.cpp.o -c /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/examples/transport_hammer.cpp

examples/CMakeFiles/transport_hammer.dir/transport_hammer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/transport_hammer.dir/transport_hammer.cpp.i"
	cd /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/examples && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/examples/transport_hammer.cpp > CMakeFiles/transport_hammer.dir/transport_hammer.cpp.i

examples/CMakeFiles/transport_hammer.dir/transport_hammer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/transport_hammer.dir/transport_hammer.cpp.s"
	cd /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/examples && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/examples/transport_hammer.cpp -o CMakeFiles/transport_hammer.dir/transport_hammer.cpp.s

examples/CMakeFiles/transport_hammer.dir/transport_hammer.cpp.o.requires:
.PHONY : examples/CMakeFiles/transport_hammer.dir/transport_hammer.cpp.o.requires

examples/CMakeFiles/transport_hammer.dir/transport_hammer.cpp.o.provides: examples/CMakeFiles/transport_hammer.dir/transport_hammer.cpp.o.requires
	$(MAKE) -f examples/CMakeFiles/transport_hammer.dir/build.make examples/CMakeFiles/transport_hammer.dir/transport_hammer.cpp.o.provides.build
.PHONY : examples/CMakeFiles/transport_hammer.dir/transport_hammer.cpp.o.provides

examples/CMakeFiles/transport_hammer.dir/transport_hammer.cpp.o.provides.build: examples/CMakeFiles/transport_hammer.dir/transport_hammer.cpp.o

# Object files for target transport_hammer
transport_hammer_OBJECTS = \
"CMakeFiles/transport_hammer.dir/transport_hammer.cpp.o"

# External object files for target transport_hammer
transport_hammer_EXTERNAL_OBJECTS =

examples/transport_hammer: examples/CMakeFiles/transport_hammer.dir/transport_hammer.cpp.o
examples/transport_hammer: examples/CMakeFiles/transport_hammer.dir/build.make
examples/transport_hammer: lib/libuhd.so.003.008
examples/transport_hammer: /usr/lib/x86_64-linux-gnu/libboost_date_time.so
examples/transport_hammer: /usr/lib/x86_64-linux-gnu/libboost_filesystem.so
examples/transport_hammer: /usr/lib/x86_64-linux-gnu/libboost_program_options.so
examples/transport_hammer: /usr/lib/x86_64-linux-gnu/libboost_regex.so
examples/transport_hammer: /usr/lib/x86_64-linux-gnu/libboost_system.so
examples/transport_hammer: /usr/lib/x86_64-linux-gnu/libboost_thread.so
examples/transport_hammer: /usr/lib/x86_64-linux-gnu/libboost_unit_test_framework.so
examples/transport_hammer: /usr/lib/x86_64-linux-gnu/libboost_serialization.so
examples/transport_hammer: /usr/lib/x86_64-linux-gnu/libpthread.so
examples/transport_hammer: /usr/lib/x86_64-linux-gnu/libusb-1.0.so
examples/transport_hammer: examples/CMakeFiles/transport_hammer.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable transport_hammer"
	cd /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/transport_hammer.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/CMakeFiles/transport_hammer.dir/build: examples/transport_hammer
.PHONY : examples/CMakeFiles/transport_hammer.dir/build

examples/CMakeFiles/transport_hammer.dir/requires: examples/CMakeFiles/transport_hammer.dir/transport_hammer.cpp.o.requires
.PHONY : examples/CMakeFiles/transport_hammer.dir/requires

examples/CMakeFiles/transport_hammer.dir/clean:
	cd /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/examples && $(CMAKE_COMMAND) -P CMakeFiles/transport_hammer.dir/cmake_clean.cmake
.PHONY : examples/CMakeFiles/transport_hammer.dir/clean

examples/CMakeFiles/transport_hammer.dir/depend:
	cd /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/examples /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/examples /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/examples/CMakeFiles/transport_hammer.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/CMakeFiles/transport_hammer.dir/depend
