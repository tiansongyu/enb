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

# Include any dependencies generated for this target.
include examples/CMakeFiles/benchmark_rate.dir/depend.make

# Include the progress variables for this target.
include examples/CMakeFiles/benchmark_rate.dir/progress.make

# Include the compile flags for this target's objects.
include examples/CMakeFiles/benchmark_rate.dir/flags.make

examples/CMakeFiles/benchmark_rate.dir/benchmark_rate.cpp.o: examples/CMakeFiles/benchmark_rate.dir/flags.make
examples/CMakeFiles/benchmark_rate.dir/benchmark_rate.cpp.o: ../examples/benchmark_rate.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object examples/CMakeFiles/benchmark_rate.dir/benchmark_rate.cpp.o"
	cd /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/examples && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/benchmark_rate.dir/benchmark_rate.cpp.o -c /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/examples/benchmark_rate.cpp

examples/CMakeFiles/benchmark_rate.dir/benchmark_rate.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/benchmark_rate.dir/benchmark_rate.cpp.i"
	cd /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/examples && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/examples/benchmark_rate.cpp > CMakeFiles/benchmark_rate.dir/benchmark_rate.cpp.i

examples/CMakeFiles/benchmark_rate.dir/benchmark_rate.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/benchmark_rate.dir/benchmark_rate.cpp.s"
	cd /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/examples && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/examples/benchmark_rate.cpp -o CMakeFiles/benchmark_rate.dir/benchmark_rate.cpp.s

examples/CMakeFiles/benchmark_rate.dir/benchmark_rate.cpp.o.requires:
.PHONY : examples/CMakeFiles/benchmark_rate.dir/benchmark_rate.cpp.o.requires

examples/CMakeFiles/benchmark_rate.dir/benchmark_rate.cpp.o.provides: examples/CMakeFiles/benchmark_rate.dir/benchmark_rate.cpp.o.requires
	$(MAKE) -f examples/CMakeFiles/benchmark_rate.dir/build.make examples/CMakeFiles/benchmark_rate.dir/benchmark_rate.cpp.o.provides.build
.PHONY : examples/CMakeFiles/benchmark_rate.dir/benchmark_rate.cpp.o.provides

examples/CMakeFiles/benchmark_rate.dir/benchmark_rate.cpp.o.provides.build: examples/CMakeFiles/benchmark_rate.dir/benchmark_rate.cpp.o

# Object files for target benchmark_rate
benchmark_rate_OBJECTS = \
"CMakeFiles/benchmark_rate.dir/benchmark_rate.cpp.o"

# External object files for target benchmark_rate
benchmark_rate_EXTERNAL_OBJECTS =

examples/benchmark_rate: examples/CMakeFiles/benchmark_rate.dir/benchmark_rate.cpp.o
examples/benchmark_rate: examples/CMakeFiles/benchmark_rate.dir/build.make
examples/benchmark_rate: lib/libuhd.so.003.009
examples/benchmark_rate: /usr/lib/x86_64-linux-gnu/libboost_date_time.so
examples/benchmark_rate: /usr/lib/x86_64-linux-gnu/libboost_filesystem.so
examples/benchmark_rate: /usr/lib/x86_64-linux-gnu/libboost_program_options.so
examples/benchmark_rate: /usr/lib/x86_64-linux-gnu/libboost_regex.so
examples/benchmark_rate: /usr/lib/x86_64-linux-gnu/libboost_system.so
examples/benchmark_rate: /usr/lib/x86_64-linux-gnu/libboost_unit_test_framework.so
examples/benchmark_rate: /usr/lib/x86_64-linux-gnu/libboost_serialization.so
examples/benchmark_rate: /usr/lib/x86_64-linux-gnu/libboost_thread.so
examples/benchmark_rate: /usr/lib/x86_64-linux-gnu/libpthread.so
examples/benchmark_rate: /usr/lib/x86_64-linux-gnu/libusb-1.0.so
examples/benchmark_rate: examples/CMakeFiles/benchmark_rate.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable benchmark_rate"
	cd /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/benchmark_rate.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/CMakeFiles/benchmark_rate.dir/build: examples/benchmark_rate
.PHONY : examples/CMakeFiles/benchmark_rate.dir/build

examples/CMakeFiles/benchmark_rate.dir/requires: examples/CMakeFiles/benchmark_rate.dir/benchmark_rate.cpp.o.requires
.PHONY : examples/CMakeFiles/benchmark_rate.dir/requires

examples/CMakeFiles/benchmark_rate.dir/clean:
	cd /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/examples && $(CMAKE_COMMAND) -P CMakeFiles/benchmark_rate.dir/cmake_clean.cmake
.PHONY : examples/CMakeFiles/benchmark_rate.dir/clean

examples/CMakeFiles/benchmark_rate.dir/depend:
	cd /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/examples /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/examples /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/examples/CMakeFiles/benchmark_rate.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/CMakeFiles/benchmark_rate.dir/depend

