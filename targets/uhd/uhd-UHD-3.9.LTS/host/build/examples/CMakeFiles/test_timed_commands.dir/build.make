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
include examples/CMakeFiles/test_timed_commands.dir/depend.make

# Include the progress variables for this target.
include examples/CMakeFiles/test_timed_commands.dir/progress.make

# Include the compile flags for this target's objects.
include examples/CMakeFiles/test_timed_commands.dir/flags.make

examples/CMakeFiles/test_timed_commands.dir/test_timed_commands.cpp.o: examples/CMakeFiles/test_timed_commands.dir/flags.make
examples/CMakeFiles/test_timed_commands.dir/test_timed_commands.cpp.o: ../examples/test_timed_commands.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object examples/CMakeFiles/test_timed_commands.dir/test_timed_commands.cpp.o"
	cd /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/examples && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/test_timed_commands.dir/test_timed_commands.cpp.o -c /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/examples/test_timed_commands.cpp

examples/CMakeFiles/test_timed_commands.dir/test_timed_commands.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_timed_commands.dir/test_timed_commands.cpp.i"
	cd /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/examples && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/examples/test_timed_commands.cpp > CMakeFiles/test_timed_commands.dir/test_timed_commands.cpp.i

examples/CMakeFiles/test_timed_commands.dir/test_timed_commands.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_timed_commands.dir/test_timed_commands.cpp.s"
	cd /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/examples && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/examples/test_timed_commands.cpp -o CMakeFiles/test_timed_commands.dir/test_timed_commands.cpp.s

examples/CMakeFiles/test_timed_commands.dir/test_timed_commands.cpp.o.requires:
.PHONY : examples/CMakeFiles/test_timed_commands.dir/test_timed_commands.cpp.o.requires

examples/CMakeFiles/test_timed_commands.dir/test_timed_commands.cpp.o.provides: examples/CMakeFiles/test_timed_commands.dir/test_timed_commands.cpp.o.requires
	$(MAKE) -f examples/CMakeFiles/test_timed_commands.dir/build.make examples/CMakeFiles/test_timed_commands.dir/test_timed_commands.cpp.o.provides.build
.PHONY : examples/CMakeFiles/test_timed_commands.dir/test_timed_commands.cpp.o.provides

examples/CMakeFiles/test_timed_commands.dir/test_timed_commands.cpp.o.provides.build: examples/CMakeFiles/test_timed_commands.dir/test_timed_commands.cpp.o

# Object files for target test_timed_commands
test_timed_commands_OBJECTS = \
"CMakeFiles/test_timed_commands.dir/test_timed_commands.cpp.o"

# External object files for target test_timed_commands
test_timed_commands_EXTERNAL_OBJECTS =

examples/test_timed_commands: examples/CMakeFiles/test_timed_commands.dir/test_timed_commands.cpp.o
examples/test_timed_commands: examples/CMakeFiles/test_timed_commands.dir/build.make
examples/test_timed_commands: lib/libuhd.so.003.009
examples/test_timed_commands: /usr/lib/x86_64-linux-gnu/libboost_date_time.so
examples/test_timed_commands: /usr/lib/x86_64-linux-gnu/libboost_filesystem.so
examples/test_timed_commands: /usr/lib/x86_64-linux-gnu/libboost_program_options.so
examples/test_timed_commands: /usr/lib/x86_64-linux-gnu/libboost_regex.so
examples/test_timed_commands: /usr/lib/x86_64-linux-gnu/libboost_system.so
examples/test_timed_commands: /usr/lib/x86_64-linux-gnu/libboost_unit_test_framework.so
examples/test_timed_commands: /usr/lib/x86_64-linux-gnu/libboost_serialization.so
examples/test_timed_commands: /usr/lib/x86_64-linux-gnu/libboost_thread.so
examples/test_timed_commands: /usr/lib/x86_64-linux-gnu/libpthread.so
examples/test_timed_commands: /usr/lib/x86_64-linux-gnu/libusb-1.0.so
examples/test_timed_commands: examples/CMakeFiles/test_timed_commands.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable test_timed_commands"
	cd /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_timed_commands.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/CMakeFiles/test_timed_commands.dir/build: examples/test_timed_commands
.PHONY : examples/CMakeFiles/test_timed_commands.dir/build

examples/CMakeFiles/test_timed_commands.dir/requires: examples/CMakeFiles/test_timed_commands.dir/test_timed_commands.cpp.o.requires
.PHONY : examples/CMakeFiles/test_timed_commands.dir/requires

examples/CMakeFiles/test_timed_commands.dir/clean:
	cd /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/examples && $(CMAKE_COMMAND) -P CMakeFiles/test_timed_commands.dir/cmake_clean.cmake
.PHONY : examples/CMakeFiles/test_timed_commands.dir/clean

examples/CMakeFiles/test_timed_commands.dir/depend:
	cd /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/examples /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/examples /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/examples/CMakeFiles/test_timed_commands.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/CMakeFiles/test_timed_commands.dir/depend

