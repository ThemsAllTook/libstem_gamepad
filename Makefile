PROJECT_NAME = gamepad

# VERSION_MAJOR, VERSION_MINOR, and VERSION_TWEAK are available as preprocessor macros for all source files in the project
VERSION_MAJOR = 1
VERSION_MINOR = 4
VERSION_TWEAK = 2
PROJECT_VERSION = ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_TWEAK}

LIBRARY_TARGETS = library
EXECUTABLE_TARGETS = 
APPLICATION_TARGETS = testharness

# TARGET_NAME_${target} required for each target of any type; HUMAN_READABLE_TARGET_NAME_${target} required for each application target. Default values for TARGET_NAME_* shown below.
#TARGET_NAME_library = libstem_${PROJECT_NAME}
#TARGET_NAME_unittest = ${PROJECT_NAME}_unittest
#TARGET_NAME_testharness = ${PROJECT_NAME}_testharness
HUMAN_READABLE_TARGET_NAME_testharness = Gamepad\ Test\ Harness

# Patterns: PLATFORMS, PLATFORMS_${target}
PLATFORMS = macosx linux32 linux64 win32 win64

DX9_INCLUDE_PATH ?= C:/MinGW/dx9/include
DX9_LIB_PATH ?= C:/MinGW/dx9/lib
DX9_LIB_PATH_i386 ?= ${DX9_LIB_PATH}/x86
DX9_LIB_PATH_x86_64 ?= ${DX9_LIB_PATH}/x64
WMI_LIB_PATH_i386 ?= C:/MinGW/WinSDK/Lib
WMI_LIB_PATH_x86_64 ?= C:/MinGW/WinSDK/Lib/x64

# Patterns: CCFLAGS, CCFLAGS_${target}, CCFLAGS_${configuration}, CCFLAGS_${platform}, CCFLAGS_${target}_${configuration}, CCFLAGS_${target}_${platform}, CCFLAGS_${configuration}_${platform}, CCFLAGS_${target}_${configuration}_${platform}
CCFLAGS_win32 = -DFREEGLUT_STATIC -I ${DX9_INCLUDE_PATH}
CCFLAGS_win64 = -DFREEGLUT_STATIC -I ${DX9_INCLUDE_PATH}

# Patterns: LINKFLAGS, LINKFLAGS_${target}, LINKFLAGS_${platform}, LINKFLAGS_${target}_${platform}
LINKFLAGS_library_macosx = -framework IOKit
LINKFLAGS_library_linux32 = -lpthread
LINKFLAGS_library_linux64 = -lpthread
LINKFLAGS_library_win32 = ${DX9_LIB_PATH_i386}/Xinput.lib ${DX9_LIB_PATH_i386}/dinput8.lib ${DX9_LIB_PATH_i386}/dxguid.lib ${WMI_LIB_PATH_i386}/WbemUuid.Lib ${WMI_LIB_PATH_i386}/Ole32.Lib ${WMI_LIB_PATH_i386}/OleAut32.Lib
LINKFLAGS_library_win64 = ${DX9_LIB_PATH_x86_64}/Xinput.lib ${DX9_LIB_PATH_x86_64}/dinput8.lib ${DX9_LIB_PATH_x86_64}/dxguid.lib ${WMI_LIB_PATH_x86_64}/WbemUuid.Lib ${WMI_LIB_PATH_x86_64}/Ole32.Lib ${WMI_LIB_PATH_x86_64}/OleAut32.Lib
LINKFLAGS_testharness_macosx = -framework CoreFoundation -framework OpenGL -framework GLUT -framework ApplicationServices
LINKFLAGS_testharness_linux32 = -lglut -lGLU -lGL
LINKFLAGS_testharness_linux64 = -lglut -lGLU -lGL
LINKFLAGS_testharness_win32 = -lfreeglut32_static -lopengl32 -lglu32 -lpthread -lwinmm -lgdi32
LINKFLAGS_testharness_win64 = -lfreeglut64_static -lopengl32 -lglu32 -lpthread -lwinmm -lgdi32

# PROJECT_LIBRARY_DEPENDENCIES_* refers to entries in LIBRARY_TARGETS to be linked when building the specified target
# Patterns: PROJECT_LIBRARY_DEPENDENCIES_${target}, PROJECT_LIBRARY_DEPENDENCIES_${target}_${platform}
PROJECT_LIBRARY_DEPENDENCIES_testharness = library

# STEM_LIBRARY_DEPENDENCIES is specified as ${PROJECT_NAME}/${PROJECT_VERSION} for each stem library to be linked when building
# Patterns: STEM_LIBRARY_DEPENDENCIES, STEM_LIBRARY_DEPENDENCIES_${target}, STEM_LIBRARY_DEPENDENCIES_${platform}, STEM_LIBRARY_DEPENDENCIES_${target}_${platform}
STEM_LIBRARY_DEPENDENCIES = 

# THIRDPARTY_LIBRARY_DEPENDENCIES is specified as ${PROJECT_NAME}/${PROJECT_VERSION} for each thirdparty library to be linked when building
# Patterns: THIRDPARTY_LIBRARY_DEPENDENCIES, THIRDPARTY_LIBRARY_DEPENDENCIES_${target}, THIRDPARTY_LIBRARY_DEPENDENCIES_${platform}, THIRDPARTY_LIBRARY_DEPENDENCIES_${target}_${platform}
THIRDPARTY_LIBRARY_DEPENDENCIES = 

# Additional build prerequisites per target
# Patterns: PREREQS, PREREQS_${target}
PREREQS = 

SOURCES_library = \
	source/${PROJECT_NAME}/Gamepad_private.c

SOURCES_library_macosx = \
	source/${PROJECT_NAME}/Gamepad_macosx.c

SOURCES_library_win32 = \
	source/${PROJECT_NAME}/Gamepad_windows_dinput.c

SOURCES_library_win64 = \
	source/${PROJECT_NAME}/Gamepad_windows_dinput.c

SOURCES_library_linux32 = \
	source/${PROJECT_NAME}/Gamepad_linux.c

SOURCES_library_linux64 = \
	source/${PROJECT_NAME}/Gamepad_linux.c

SOURCES_testharness = \
	source/testharness/TestHarness_main.c

# Public-facing include files to be distributed with the library, if any
INCLUDES = \
	source/${PROJECT_NAME}/Gamepad.h

# Patterns: RESOURCES, RESOURCES_${target}, RESOURCES_${platform}, RESOURCES_${target}_${platform}
RESOURCES = 

# Pattern: ANALYZER_EXCLUDE_SOURCES_${analyzer}
ANALYZER_EXCLUDE_SOURCES_clang = 
ANALYZER_EXCLUDE_SOURCES_splint = ${SOURCES_unittest_suites}

# Pattern: PLIST_FILE_${target}_${platform}
PLIST_FILE_testharness_macosx = resources/Info_testharness_macosx.plist

INSTALLED_TARGETS = library testharness

include Makefile.global
