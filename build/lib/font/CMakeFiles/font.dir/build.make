# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.20

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\CMake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\CMake\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\VSARM\sdk\pico\pico-projects\HoffSolo

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\VSARM\sdk\pico\pico-projects\HoffSolo\build

# Include any dependencies generated for this target.
include lib/font/CMakeFiles/font.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include lib/font/CMakeFiles/font.dir/compiler_depend.make

# Include the progress variables for this target.
include lib/font/CMakeFiles/font.dir/progress.make

# Include the compile flags for this target's objects.
include lib/font/CMakeFiles/font.dir/flags.make

lib/font/CMakeFiles/font.dir/font12.c.obj: lib/font/CMakeFiles/font.dir/flags.make
lib/font/CMakeFiles/font.dir/font12.c.obj: ../lib/font/font12.c
lib/font/CMakeFiles/font.dir/font12.c.obj: lib/font/CMakeFiles/font.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object lib/font/CMakeFiles/font.dir/font12.c.obj"
	cd /d C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\lib\font && C:\VSARM\armgcc\112202~1.02\bin\AR19DD~1.EXE $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lib/font/CMakeFiles/font.dir/font12.c.obj -MF CMakeFiles\font.dir\font12.c.obj.d -o CMakeFiles\font.dir\font12.c.obj -c C:\VSARM\sdk\pico\pico-projects\HoffSolo\lib\font\font12.c

lib/font/CMakeFiles/font.dir/font12.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/font.dir/font12.c.i"
	cd /d C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\lib\font && C:\VSARM\armgcc\112202~1.02\bin\AR19DD~1.EXE $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E C:\VSARM\sdk\pico\pico-projects\HoffSolo\lib\font\font12.c > CMakeFiles\font.dir\font12.c.i

lib/font/CMakeFiles/font.dir/font12.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/font.dir/font12.c.s"
	cd /d C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\lib\font && C:\VSARM\armgcc\112202~1.02\bin\AR19DD~1.EXE $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S C:\VSARM\sdk\pico\pico-projects\HoffSolo\lib\font\font12.c -o CMakeFiles\font.dir\font12.c.s

lib/font/CMakeFiles/font.dir/font16.c.obj: lib/font/CMakeFiles/font.dir/flags.make
lib/font/CMakeFiles/font.dir/font16.c.obj: ../lib/font/font16.c
lib/font/CMakeFiles/font.dir/font16.c.obj: lib/font/CMakeFiles/font.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object lib/font/CMakeFiles/font.dir/font16.c.obj"
	cd /d C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\lib\font && C:\VSARM\armgcc\112202~1.02\bin\AR19DD~1.EXE $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lib/font/CMakeFiles/font.dir/font16.c.obj -MF CMakeFiles\font.dir\font16.c.obj.d -o CMakeFiles\font.dir\font16.c.obj -c C:\VSARM\sdk\pico\pico-projects\HoffSolo\lib\font\font16.c

lib/font/CMakeFiles/font.dir/font16.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/font.dir/font16.c.i"
	cd /d C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\lib\font && C:\VSARM\armgcc\112202~1.02\bin\AR19DD~1.EXE $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E C:\VSARM\sdk\pico\pico-projects\HoffSolo\lib\font\font16.c > CMakeFiles\font.dir\font16.c.i

lib/font/CMakeFiles/font.dir/font16.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/font.dir/font16.c.s"
	cd /d C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\lib\font && C:\VSARM\armgcc\112202~1.02\bin\AR19DD~1.EXE $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S C:\VSARM\sdk\pico\pico-projects\HoffSolo\lib\font\font16.c -o CMakeFiles\font.dir\font16.c.s

lib/font/CMakeFiles/font.dir/font20.c.obj: lib/font/CMakeFiles/font.dir/flags.make
lib/font/CMakeFiles/font.dir/font20.c.obj: ../lib/font/font20.c
lib/font/CMakeFiles/font.dir/font20.c.obj: lib/font/CMakeFiles/font.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object lib/font/CMakeFiles/font.dir/font20.c.obj"
	cd /d C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\lib\font && C:\VSARM\armgcc\112202~1.02\bin\AR19DD~1.EXE $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lib/font/CMakeFiles/font.dir/font20.c.obj -MF CMakeFiles\font.dir\font20.c.obj.d -o CMakeFiles\font.dir\font20.c.obj -c C:\VSARM\sdk\pico\pico-projects\HoffSolo\lib\font\font20.c

lib/font/CMakeFiles/font.dir/font20.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/font.dir/font20.c.i"
	cd /d C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\lib\font && C:\VSARM\armgcc\112202~1.02\bin\AR19DD~1.EXE $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E C:\VSARM\sdk\pico\pico-projects\HoffSolo\lib\font\font20.c > CMakeFiles\font.dir\font20.c.i

lib/font/CMakeFiles/font.dir/font20.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/font.dir/font20.c.s"
	cd /d C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\lib\font && C:\VSARM\armgcc\112202~1.02\bin\AR19DD~1.EXE $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S C:\VSARM\sdk\pico\pico-projects\HoffSolo\lib\font\font20.c -o CMakeFiles\font.dir\font20.c.s

lib/font/CMakeFiles/font.dir/font24.c.obj: lib/font/CMakeFiles/font.dir/flags.make
lib/font/CMakeFiles/font.dir/font24.c.obj: ../lib/font/font24.c
lib/font/CMakeFiles/font.dir/font24.c.obj: lib/font/CMakeFiles/font.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object lib/font/CMakeFiles/font.dir/font24.c.obj"
	cd /d C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\lib\font && C:\VSARM\armgcc\112202~1.02\bin\AR19DD~1.EXE $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lib/font/CMakeFiles/font.dir/font24.c.obj -MF CMakeFiles\font.dir\font24.c.obj.d -o CMakeFiles\font.dir\font24.c.obj -c C:\VSARM\sdk\pico\pico-projects\HoffSolo\lib\font\font24.c

lib/font/CMakeFiles/font.dir/font24.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/font.dir/font24.c.i"
	cd /d C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\lib\font && C:\VSARM\armgcc\112202~1.02\bin\AR19DD~1.EXE $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E C:\VSARM\sdk\pico\pico-projects\HoffSolo\lib\font\font24.c > CMakeFiles\font.dir\font24.c.i

lib/font/CMakeFiles/font.dir/font24.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/font.dir/font24.c.s"
	cd /d C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\lib\font && C:\VSARM\armgcc\112202~1.02\bin\AR19DD~1.EXE $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S C:\VSARM\sdk\pico\pico-projects\HoffSolo\lib\font\font24.c -o CMakeFiles\font.dir\font24.c.s

lib/font/CMakeFiles/font.dir/font8.c.obj: lib/font/CMakeFiles/font.dir/flags.make
lib/font/CMakeFiles/font.dir/font8.c.obj: ../lib/font/font8.c
lib/font/CMakeFiles/font.dir/font8.c.obj: lib/font/CMakeFiles/font.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object lib/font/CMakeFiles/font.dir/font8.c.obj"
	cd /d C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\lib\font && C:\VSARM\armgcc\112202~1.02\bin\AR19DD~1.EXE $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lib/font/CMakeFiles/font.dir/font8.c.obj -MF CMakeFiles\font.dir\font8.c.obj.d -o CMakeFiles\font.dir\font8.c.obj -c C:\VSARM\sdk\pico\pico-projects\HoffSolo\lib\font\font8.c

lib/font/CMakeFiles/font.dir/font8.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/font.dir/font8.c.i"
	cd /d C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\lib\font && C:\VSARM\armgcc\112202~1.02\bin\AR19DD~1.EXE $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E C:\VSARM\sdk\pico\pico-projects\HoffSolo\lib\font\font8.c > CMakeFiles\font.dir\font8.c.i

lib/font/CMakeFiles/font.dir/font8.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/font.dir/font8.c.s"
	cd /d C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\lib\font && C:\VSARM\armgcc\112202~1.02\bin\AR19DD~1.EXE $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S C:\VSARM\sdk\pico\pico-projects\HoffSolo\lib\font\font8.c -o CMakeFiles\font.dir\font8.c.s

# Object files for target font
font_OBJECTS = \
"CMakeFiles/font.dir/font12.c.obj" \
"CMakeFiles/font.dir/font16.c.obj" \
"CMakeFiles/font.dir/font20.c.obj" \
"CMakeFiles/font.dir/font24.c.obj" \
"CMakeFiles/font.dir/font8.c.obj"

# External object files for target font
font_EXTERNAL_OBJECTS =

lib/font/libfont.a: lib/font/CMakeFiles/font.dir/font12.c.obj
lib/font/libfont.a: lib/font/CMakeFiles/font.dir/font16.c.obj
lib/font/libfont.a: lib/font/CMakeFiles/font.dir/font20.c.obj
lib/font/libfont.a: lib/font/CMakeFiles/font.dir/font24.c.obj
lib/font/libfont.a: lib/font/CMakeFiles/font.dir/font8.c.obj
lib/font/libfont.a: lib/font/CMakeFiles/font.dir/build.make
lib/font/libfont.a: lib/font/CMakeFiles/font.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking C static library libfont.a"
	cd /d C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\lib\font && $(CMAKE_COMMAND) -P CMakeFiles\font.dir\cmake_clean_target.cmake
	cd /d C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\lib\font && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\font.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
lib/font/CMakeFiles/font.dir/build: lib/font/libfont.a
.PHONY : lib/font/CMakeFiles/font.dir/build

lib/font/CMakeFiles/font.dir/clean:
	cd /d C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\lib\font && $(CMAKE_COMMAND) -P CMakeFiles\font.dir\cmake_clean.cmake
.PHONY : lib/font/CMakeFiles/font.dir/clean

lib/font/CMakeFiles/font.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\VSARM\sdk\pico\pico-projects\HoffSolo C:\VSARM\sdk\pico\pico-projects\HoffSolo\lib\font C:\VSARM\sdk\pico\pico-projects\HoffSolo\build C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\lib\font C:\VSARM\sdk\pico\pico-projects\HoffSolo\build\lib\font\CMakeFiles\font.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : lib/font/CMakeFiles/font.dir/depend

