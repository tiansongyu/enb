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
include tests/CMakeFiles/chdr_test.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/chdr_test.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/chdr_test.dir/flags.make

tests/CMakeFiles/chdr_test.dir/chdr_test.cpp.o: tests/CMakeFiles/chdr_test.dir/flags.make
tests/CMakeFiles/chdr_test.dir/chdr_test.cpp.o: ../tests/chdr_test.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object tests/CMakeFiles/chdr_test.dir/chdr_test.cpp.o"
	cd /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/tests && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/chdr_test.dir/chdr_test.cpp.o -c /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/tests/chdr_test.cpp

tests/CMakeFiles/chdr_test.dir/chdr_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/chdr_test.dir/chdr_test.cpp.i"
	cd /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/tests && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/tests/chdr_test.cpp > CMakeFiles/chdr_test.dir/chdr_test.cpp.i

tests/CMakeFiles/chdr_test.dir/chdr_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/chdr_test.dir/chdr_test.cpp.s"
	cd /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/tests && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/tests/chdr_test.cpp -o CMakeFiles/chdr_test.dir/chdr_test.cpp.s

tests/CMakeFiles/chdr_test.dir/chdr_test.cpp.o.requires:
.PHONY : tests/CMakeFiles/chdr_test.dir/chdr_test.cpp.o.requires

tests/CMakeFiles/chdr_test.dir/chdr_test.cpp.o.provides: tests/CMakeFiles/chdr_test.dir/chdr_test.cpp.o.requires
	$(MAKE) -f tests/CMakeFiles/chdr_test.dir/build.make tests/CMakeFiles/chdr_test.dir/chdr_test.cpp.o.provides.build
.PHONY : tests/CMakeFiles/chdr_test.dir/chdr_test.cpp.o.provides

tests/CMakeFiles/chdr_test.dir/chdr_test.cpp.o.provides.build: tests/CMakeFiles/chdr_test.dir/chdr_test.cpp.o

# Object files for target chdr_test
chdr_test_OBJECTS = \
"CMakeFiles/chdr_test.dir/chdr_test.cpp.o"

# External object files for target chdr_test
chdr_test_EXTERNAL_OBJECTS =

tests/chdr_test: tests/CMakeFiles/chdr_test.dir/chdr_test.cpp.o
tests/chdr_test: tests/CMakeFiles/chdr_test.dir/build.make
tests/chdr_test: lib/libuhd.so.003.009
tests/chdr_test: /usr/lib/x86_64-linux-gnu/libboost_date_time.so
tests/chdr_test: /usr/lib/x86_64-linux-gnu/libboost_filesystem.so
tests/chdr_test: /usr/lib/x86_64-linux-gnu/libboost_program_options.so
tests/chdr_test: /usr/lib/x86_64-linux-gnu/libboost_regex.so
tests/chdr_test: /usr/lib/x86_64-linux-gnu/libboost_system.so
tests/chdr_test: /usr/lib/x86_64-linux-gnu/libboost_unit_test_framework.so
tests/chdr_test: /usr/lib/x86_64-linux-gnu/libboost_serialization.so
tests/chdr_test: /usr/lib/x86_64-linux-gnu/libboost_thread.so
tests/chdr_test: /usr/lib/x86_64-linux-gnu/libpthread.so
tests/chdr_test: /usr/lib/x86_64-linux-gnu/libusb-1.0.so
tests/chdr_test: tests/CMakeFiles/chdr_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable chdr_test"
	cd /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/chdr_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/chdr_test.dir/build: tests/chdr_test
.PHONY : tests/CMakeFiles/chdr_test.dir/build

tests/CMakeFiles/chdr_test.dir/requires: tests/CMakeFiles/chdr_test.dir/chdr_test.cpp.o.requires
.PHONY : tests/CMakeFiles/chdr_test.dir/requires

tests/CMakeFiles/chdr_test.dir/clean:
	cd /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/tests && $(CMAKE_COMMAND) -P CMakeFiles/chdr_test.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/chdr_test.dir/clean

tests/CMakeFiles/chdr_test.dir/depend:
	cd /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/tests /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/tests /home/oai/trunk_enb/targets/uhd/uhd-UHD-3.9.LTS/host/build/tests/CMakeFiles/chdr_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/chdr_test.dir/depend

