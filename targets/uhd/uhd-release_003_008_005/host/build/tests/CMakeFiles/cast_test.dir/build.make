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
include tests/CMakeFiles/cast_test.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/cast_test.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/cast_test.dir/flags.make

tests/CMakeFiles/cast_test.dir/cast_test.cpp.o: tests/CMakeFiles/cast_test.dir/flags.make
tests/CMakeFiles/cast_test.dir/cast_test.cpp.o: ../tests/cast_test.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object tests/CMakeFiles/cast_test.dir/cast_test.cpp.o"
	cd /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/tests && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/cast_test.dir/cast_test.cpp.o -c /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/tests/cast_test.cpp

tests/CMakeFiles/cast_test.dir/cast_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cast_test.dir/cast_test.cpp.i"
	cd /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/tests && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/tests/cast_test.cpp > CMakeFiles/cast_test.dir/cast_test.cpp.i

tests/CMakeFiles/cast_test.dir/cast_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cast_test.dir/cast_test.cpp.s"
	cd /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/tests && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/tests/cast_test.cpp -o CMakeFiles/cast_test.dir/cast_test.cpp.s

tests/CMakeFiles/cast_test.dir/cast_test.cpp.o.requires:
.PHONY : tests/CMakeFiles/cast_test.dir/cast_test.cpp.o.requires

tests/CMakeFiles/cast_test.dir/cast_test.cpp.o.provides: tests/CMakeFiles/cast_test.dir/cast_test.cpp.o.requires
	$(MAKE) -f tests/CMakeFiles/cast_test.dir/build.make tests/CMakeFiles/cast_test.dir/cast_test.cpp.o.provides.build
.PHONY : tests/CMakeFiles/cast_test.dir/cast_test.cpp.o.provides

tests/CMakeFiles/cast_test.dir/cast_test.cpp.o.provides.build: tests/CMakeFiles/cast_test.dir/cast_test.cpp.o

# Object files for target cast_test
cast_test_OBJECTS = \
"CMakeFiles/cast_test.dir/cast_test.cpp.o"

# External object files for target cast_test
cast_test_EXTERNAL_OBJECTS =

tests/cast_test: tests/CMakeFiles/cast_test.dir/cast_test.cpp.o
tests/cast_test: tests/CMakeFiles/cast_test.dir/build.make
tests/cast_test: lib/libuhd.so.003.008
tests/cast_test: /usr/lib/x86_64-linux-gnu/libboost_date_time.so
tests/cast_test: /usr/lib/x86_64-linux-gnu/libboost_filesystem.so
tests/cast_test: /usr/lib/x86_64-linux-gnu/libboost_program_options.so
tests/cast_test: /usr/lib/x86_64-linux-gnu/libboost_regex.so
tests/cast_test: /usr/lib/x86_64-linux-gnu/libboost_system.so
tests/cast_test: /usr/lib/x86_64-linux-gnu/libboost_thread.so
tests/cast_test: /usr/lib/x86_64-linux-gnu/libboost_unit_test_framework.so
tests/cast_test: /usr/lib/x86_64-linux-gnu/libboost_serialization.so
tests/cast_test: /usr/lib/x86_64-linux-gnu/libpthread.so
tests/cast_test: /usr/lib/x86_64-linux-gnu/libusb-1.0.so
tests/cast_test: tests/CMakeFiles/cast_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable cast_test"
	cd /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cast_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/cast_test.dir/build: tests/cast_test
.PHONY : tests/CMakeFiles/cast_test.dir/build

tests/CMakeFiles/cast_test.dir/requires: tests/CMakeFiles/cast_test.dir/cast_test.cpp.o.requires
.PHONY : tests/CMakeFiles/cast_test.dir/requires

tests/CMakeFiles/cast_test.dir/clean:
	cd /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/tests && $(CMAKE_COMMAND) -P CMakeFiles/cast_test.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/cast_test.dir/clean

tests/CMakeFiles/cast_test.dir/depend:
	cd /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/tests /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/tests /home/oai/trunk_enb/targets/uhd/uhd-release_003_008_005/host/build/tests/CMakeFiles/cast_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/cast_test.dir/depend

