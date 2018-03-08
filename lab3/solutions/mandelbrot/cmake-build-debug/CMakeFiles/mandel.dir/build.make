# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.9

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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
CMAKE_COMMAND = /opt/clion/bin/cmake/bin/cmake

# The command to remove a file.
RM = /opt/clion/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ishtar/mountpoint/lab3/solutions/mandelbrot

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ishtar/mountpoint/lab3/solutions/mandelbrot/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/mandel.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/mandel.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/mandel.dir/flags.make

CMakeFiles/mandel.dir/mandel.c.o: CMakeFiles/mandel.dir/flags.make
CMakeFiles/mandel.dir/mandel.c.o: ../mandel.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ishtar/mountpoint/lab3/solutions/mandelbrot/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/mandel.dir/mandel.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/mandel.dir/mandel.c.o   -c /home/ishtar/mountpoint/lab3/solutions/mandelbrot/mandel.c

CMakeFiles/mandel.dir/mandel.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/mandel.dir/mandel.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/ishtar/mountpoint/lab3/solutions/mandelbrot/mandel.c > CMakeFiles/mandel.dir/mandel.c.i

CMakeFiles/mandel.dir/mandel.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/mandel.dir/mandel.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/ishtar/mountpoint/lab3/solutions/mandelbrot/mandel.c -o CMakeFiles/mandel.dir/mandel.c.s

CMakeFiles/mandel.dir/mandel.c.o.requires:

.PHONY : CMakeFiles/mandel.dir/mandel.c.o.requires

CMakeFiles/mandel.dir/mandel.c.o.provides: CMakeFiles/mandel.dir/mandel.c.o.requires
	$(MAKE) -f CMakeFiles/mandel.dir/build.make CMakeFiles/mandel.dir/mandel.c.o.provides.build
.PHONY : CMakeFiles/mandel.dir/mandel.c.o.provides

CMakeFiles/mandel.dir/mandel.c.o.provides.build: CMakeFiles/mandel.dir/mandel.c.o


CMakeFiles/mandel.dir/mandel-lib.c.o: CMakeFiles/mandel.dir/flags.make
CMakeFiles/mandel.dir/mandel-lib.c.o: ../mandel-lib.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ishtar/mountpoint/lab3/solutions/mandelbrot/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/mandel.dir/mandel-lib.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/mandel.dir/mandel-lib.c.o   -c /home/ishtar/mountpoint/lab3/solutions/mandelbrot/mandel-lib.c

CMakeFiles/mandel.dir/mandel-lib.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/mandel.dir/mandel-lib.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/ishtar/mountpoint/lab3/solutions/mandelbrot/mandel-lib.c > CMakeFiles/mandel.dir/mandel-lib.c.i

CMakeFiles/mandel.dir/mandel-lib.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/mandel.dir/mandel-lib.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/ishtar/mountpoint/lab3/solutions/mandelbrot/mandel-lib.c -o CMakeFiles/mandel.dir/mandel-lib.c.s

CMakeFiles/mandel.dir/mandel-lib.c.o.requires:

.PHONY : CMakeFiles/mandel.dir/mandel-lib.c.o.requires

CMakeFiles/mandel.dir/mandel-lib.c.o.provides: CMakeFiles/mandel.dir/mandel-lib.c.o.requires
	$(MAKE) -f CMakeFiles/mandel.dir/build.make CMakeFiles/mandel.dir/mandel-lib.c.o.provides.build
.PHONY : CMakeFiles/mandel.dir/mandel-lib.c.o.provides

CMakeFiles/mandel.dir/mandel-lib.c.o.provides.build: CMakeFiles/mandel.dir/mandel-lib.c.o


# Object files for target mandel
mandel_OBJECTS = \
"CMakeFiles/mandel.dir/mandel.c.o" \
"CMakeFiles/mandel.dir/mandel-lib.c.o"

# External object files for target mandel
mandel_EXTERNAL_OBJECTS =

mandel: CMakeFiles/mandel.dir/mandel.c.o
mandel: CMakeFiles/mandel.dir/mandel-lib.c.o
mandel: CMakeFiles/mandel.dir/build.make
mandel: CMakeFiles/mandel.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ishtar/mountpoint/lab3/solutions/mandelbrot/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable mandel"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/mandel.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/mandel.dir/build: mandel

.PHONY : CMakeFiles/mandel.dir/build

CMakeFiles/mandel.dir/requires: CMakeFiles/mandel.dir/mandel.c.o.requires
CMakeFiles/mandel.dir/requires: CMakeFiles/mandel.dir/mandel-lib.c.o.requires

.PHONY : CMakeFiles/mandel.dir/requires

CMakeFiles/mandel.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/mandel.dir/cmake_clean.cmake
.PHONY : CMakeFiles/mandel.dir/clean

CMakeFiles/mandel.dir/depend:
	cd /home/ishtar/mountpoint/lab3/solutions/mandelbrot/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ishtar/mountpoint/lab3/solutions/mandelbrot /home/ishtar/mountpoint/lab3/solutions/mandelbrot /home/ishtar/mountpoint/lab3/solutions/mandelbrot/cmake-build-debug /home/ishtar/mountpoint/lab3/solutions/mandelbrot/cmake-build-debug /home/ishtar/mountpoint/lab3/solutions/mandelbrot/cmake-build-debug/CMakeFiles/mandel.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/mandel.dir/depend

