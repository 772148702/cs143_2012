# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt/e/OneDrive/public_lessons/cs143/pp3

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/e/OneDrive/public_lessons/cs143/pp3/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/y.tab.c.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/y.tab.c.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/y.tab.c.dir/flags.make

CMakeFiles/y.tab.c.dir/lex.yy.c.o: CMakeFiles/y.tab.c.dir/flags.make
CMakeFiles/y.tab.c.dir/lex.yy.c.o: ../lex.yy.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/e/OneDrive/public_lessons/cs143/pp3/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/y.tab.c.dir/lex.yy.c.o"
	/usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/y.tab.c.dir/lex.yy.c.o   -c /mnt/e/OneDrive/public_lessons/cs143/pp3/lex.yy.c

CMakeFiles/y.tab.c.dir/lex.yy.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/y.tab.c.dir/lex.yy.c.i"
	/usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/e/OneDrive/public_lessons/cs143/pp3/lex.yy.c > CMakeFiles/y.tab.c.dir/lex.yy.c.i

CMakeFiles/y.tab.c.dir/lex.yy.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/y.tab.c.dir/lex.yy.c.s"
	/usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/e/OneDrive/public_lessons/cs143/pp3/lex.yy.c -o CMakeFiles/y.tab.c.dir/lex.yy.c.s

CMakeFiles/y.tab.c.dir/lex.yy.c.o.requires:

.PHONY : CMakeFiles/y.tab.c.dir/lex.yy.c.o.requires

CMakeFiles/y.tab.c.dir/lex.yy.c.o.provides: CMakeFiles/y.tab.c.dir/lex.yy.c.o.requires
	$(MAKE) -f CMakeFiles/y.tab.c.dir/build.make CMakeFiles/y.tab.c.dir/lex.yy.c.o.provides.build
.PHONY : CMakeFiles/y.tab.c.dir/lex.yy.c.o.provides

CMakeFiles/y.tab.c.dir/lex.yy.c.o.provides.build: CMakeFiles/y.tab.c.dir/lex.yy.c.o


# Object files for target y.tab.c
y_tab_c_OBJECTS = \
"CMakeFiles/y.tab.c.dir/lex.yy.c.o"

# External object files for target y.tab.c
y_tab_c_EXTERNAL_OBJECTS =

liby.tab.c.a: CMakeFiles/y.tab.c.dir/lex.yy.c.o
liby.tab.c.a: CMakeFiles/y.tab.c.dir/build.make
liby.tab.c.a: CMakeFiles/y.tab.c.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/e/OneDrive/public_lessons/cs143/pp3/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C static library liby.tab.c.a"
	$(CMAKE_COMMAND) -P CMakeFiles/y.tab.c.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/y.tab.c.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/y.tab.c.dir/build: liby.tab.c.a

.PHONY : CMakeFiles/y.tab.c.dir/build

CMakeFiles/y.tab.c.dir/requires: CMakeFiles/y.tab.c.dir/lex.yy.c.o.requires

.PHONY : CMakeFiles/y.tab.c.dir/requires

CMakeFiles/y.tab.c.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/y.tab.c.dir/cmake_clean.cmake
.PHONY : CMakeFiles/y.tab.c.dir/clean

CMakeFiles/y.tab.c.dir/depend:
	cd /mnt/e/OneDrive/public_lessons/cs143/pp3/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/e/OneDrive/public_lessons/cs143/pp3 /mnt/e/OneDrive/public_lessons/cs143/pp3 /mnt/e/OneDrive/public_lessons/cs143/pp3/cmake-build-debug /mnt/e/OneDrive/public_lessons/cs143/pp3/cmake-build-debug /mnt/e/OneDrive/public_lessons/cs143/pp3/cmake-build-debug/CMakeFiles/y.tab.c.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/y.tab.c.dir/depend
