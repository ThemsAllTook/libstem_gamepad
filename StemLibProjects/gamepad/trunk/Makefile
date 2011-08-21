.PHONY: all
all: library unittest testharness test include

UNAME = ${shell uname}
ifeq (${UNAME},Linux)
HOST_PLATFORM = linux
else ifeq (${UNAME},Darwin)
-include ~/.stem.defines
HOST_PLATFORM = macosx
else
HOST_PLATFORM = windows
endif

include version

define newline_and_tab

	
endef

iphone_sdk_version_integer = ${subst .,0,$1}${word ${words ${wordlist 2, ${words ${subst ., ,$1}}, ${subst ., ,$1}}}, 00}

TARGET_PLATFORMS_macosx = macosx iphonesimulator iphoneos
TARGET_PLATFORMS_linux = linux
TARGET_PLATFORMS_windows = windows

PROJECT_NAME = gamepad
IPHONE_BUILD_SDK_VERSION ?= 4.2
IPHONE_DEPLOYMENT_TARGET_VERSION = 3.1
CODESIGN_IDENTITY = "iPhone Developer"
SVNROOT = http://sacredsoftware.net/svn/misc#master_source_only

LIBRARY_TARGETS = library
EXECUTABLE_TARGETS = unittest
APPLICATION_TARGETS = testharness
TARGETS = ${LIBRARY_TARGETS} ${EXECUTABLE_TARGETS} ${APPLICATION_TARGETS}
CONFIGURATIONS = debug profile release
PLATFORMS = ${filter ${TARGET_PLATFORMS_${HOST_PLATFORM}},macosx linux windows}
ANALYZERS = splint clang

TARGET_NAME_library = libstem_gamepad
TARGET_NAME_unittest = gamepad_unittest
TARGET_NAME_testharness = gamepad_testharness
HUMAN_READABLE_TARGET_NAME_testharness = GamepadTestHarness

#Per-target configurations
CONFIGURATIONS_library = debug profile release
CONFIGURATIONS_unittest = debug
CONFIGURATIONS_testharness = debug profile release

#Per-target platforms
PLATFORMS_library = ${filter ${PLATFORMS},macosx linux windows}
PLATFORMS_unittest = ${filter ${PLATFORMS},macosx linux windows}
PLATFORMS_testharness = ${filter ${PLATFORMS},macosx linux windows}

#Per-target compile/link settings
CCFLAGS_unittest = -I test_source -include build/intermediate/TestList.h
CCFLAGS_testharness = -DGLEW_STATIC

#Per-target analyzer settings
CLANGFLAGS_unittest = ${CCFLAGS_unittest}
SPLINTFLAGS_unittest = ${CCFLAGS_unittest}

#Per-configuration compile/link settings
CCFLAGS_debug = -g -DDEBUG
CCFLAGS_profile = -g -O3
CCFLAGS_release = -O3

#Per-platform compile/link settings
CC_macosx_ppc = /usr/bin/gcc-4.2 -arch ppc
CC_macosx_i386 = /usr/bin/clang -arch i386
CC_macosx_x86_64 = /usr/bin/clang -arch x86_64
AR_macosx = /usr/bin/ar
RANLIB_macosx = /usr/bin/ranlib
SPLINT_macosx = /usr/local/bin/splint
CLANG_macosx = /usr/bin/clang
SDKROOT_macosx = /Developer/SDKs/MacOSX10.5.sdk
ARCHS_macosx = ppc i386 x86_64
CCFLAGS_macosx = -isysroot ${SDKROOT_macosx} -mmacosx-version-min=10.5
LINKFLAGS_macosx = -isysroot ${SDKROOT_macosx} -mmacosx-version-min=10.5 -framework IOKit -framework CoreFoundation -framework OpenGL -framework GLUT

CC_linux_i686 = /usr/bin/gcc
AR_linux = /usr/bin/ar
RANLIB_linux = /usr/bin/ranlib
SPLINT_linux = /usr/local/bin/splint
CLANG_linux = /usr/local/bin/clang
ARCHS_linux = i686
CCFLAGS_linux = 
LINKFLAGS_linux = -lm -ldl -lglut -lGLU -Wl,-E

CC_windows_i686 = C:/MinGW/bin/gcc.exe
AR_windows = C:/MinGW/bin/ar.exe
RANLIB_windows = C:/MinGW/bin/ranlib.exe
SPLINT_windows = C:/splint-3.1.1/bin/splint.exe
CLANG_windows = C:/llvm/bin/clang.exe
ARCHS_windows = i686
CCFLAGS_windows = -DFREEGLUT_STATIC
LINKFLAGS_windows = -lfreeglut_static -lopengl32 -lglu32 -lpthread -lwinmm -lgdi32 -mconsole
EXECUTABLE_SUFFIX_windows = .exe

#General compile/link settings
DEFINE_CCFLAGS = -DVERSION_MAJOR=${VERSION_MAJOR}u -DVERSION_MINOR=${VERSION_MINOR}u -DVERSION_TWEAK=${VERSION_TWEAK}u
WARNING_CCFLAGS = -Wall -Wextra -Wno-unused-parameter -Werror
OTHER_CCFLAGS = -std=gnu99
CCFLAGS = ${DEFINE_CCFLAGS} ${WARNING_CCFLAGS} ${OTHER_CCFLAGS}

FRAMEWORK_LINKFLAGS = 
LIBRARY_LINKFLAGS = 
OTHER_LINKFLAGS = 
LINKFLAGS = ${FRAMEWORK_LINKFLAGS} ${LIBRARY_LINKFLAGS} ${OTHER_LINKFLAGS}

LINK_ORDER = \
	library \
	utilities/libstem_utilities.a \
	glutshell/libstemshell_glut.a \
	glgraphics/libstem_glgraphics.a \
	stemobject/libstem_stemobject.a \
	glew/libglew.a

#Per-target dependencies

PROJECT_LIBRARY_DEPENDENCIES_unittest = library
PROJECT_LIBRARY_DEPENDENCIES_testharness = library
STEM_LIBRARY_DEPENDENCIES_library = \
	utilities/libstem_utilities.a \
	stemobject/libstem_stemobject.a \
	glutshell/libstemshell_glut.a \
	glgraphics/libstem_glgraphics.a
STEM_LIBRARY_DEPENDENCIES_unittest =
STEM_LIBRARY_DEPENDENCIES_testharness = \
	utilities/libstem_utilities.a \
	stemobject/libstem_stemobject.a \
	glutshell/libstemshell_glut.a \
	glgraphics/libstem_glgraphics.a
STEM_SOURCE_DEPENDENCIES_library = 
STEM_SOURCE_DEPENDENCIES_unittest = 
STEM_SOURCE_DEPENDENCIES_testharness = 
THIRDPARTY_LIBRARY_DEPENDENCIES_library = 
THIRDPARTY_LIBRARY_DEPENDENCIES_unittest = 
THIRDPARTY_LIBRARY_DEPENDENCIES_testharness = \
	glew/libglew.a

#Per-target per-platform dependencies

PROJECT_LIBRARY_DEPENDENCIES_unittest_macosx = 
STEM_LIBRARY_DEPENDENCIES_unittest_macosx = 
STEM_SOURCE_DEPENDENCIES_unittest_macosx = 
THIRDPARTY_LIBRARY_DEPENDENCIES_unittest_macosx = 
#...

#Per-target source file lists

SOURCES_library = 

SOURCES_library_macosx = \
	source/gamepad/Gamepad_macosx.c

SOURCES_library_windows = \
	source/gamepad/Gamepad_windows.c

SOURCES_library_linux = \
	source/gamepad/Gamepad_linux.c

SOURCES_unittest = \
	test_source/unittest/framework/unittest_main.c \
	test_source/unittest/framework/TestList.c \
	${SOURCES_unittest_suites}

SOURCES_unittest_suites = 

SOURCES_testharness = \
	test_source/testharness/TestHarness_main.c

#Include files to be distributed with library

INCLUDES = \
	source/gamepad/Gamepad.h

#Target resources

RESOURCES_testharness = 
RESOURCES_unittest = 
RESOURCES_testharness_macosx = 
#...

#General analyzer settings
CLANGFLAGS = 
CLANGFLAGS_windows = -I C:/MinGW/include -I C:/MinGW/lib/gcc/mingw32/4.3.2/include
SPLINTFLAGS = -exportlocal

#Source files excluded from static analysis

ANALYZER_EXCLUDE_SOURCES_clang = 
ANALYZER_EXCLUDE_SOURCES_splint = ${SOURCES_unittest}

#Additional target build prerequisites

PREREQS_unittest = build/intermediate/TestList.h

#TestList.h is automatically generated from ${SOURCES_unittest_suites}. It is used by the unit test framework to determine which tests to run.
build/intermediate/TestList.h: build/intermediate/TestSuites.txt | build/intermediate
	echo '#define SUITE_FILE_LIST ${foreach file,${SOURCES_unittest_suites},"${basename ${notdir ${file}}}",} NULL' > $@

#TestSuites.txt tracks the state of ${SOURCES_unittest_suites} so that TestList.h can be updated if and only if ${SOURCES_unittest_suites} has changed. .PHONY is abused slightly to cause the target to be conditionally remade.
ifneq (${shell echo "${SOURCES_unittest_suites}" | cmp - build/intermediate/TestSuites.txt 2>&1},)
.PHONY: build/intermediate/TestSuites.txt
endif
build/intermediate/TestSuites.txt: | build/intermediate
	echo "${SOURCES_unittest_suites}" > $@



define configuration_object_list_template #(target, configuration)
	${foreach platform,${PLATFORMS_$1}, \
		${call platform_object_list_template,$1,$2,${platform}} \
	}
endef

define platform_object_list_template #(target, configuration, platform)
	${foreach arch,${ARCHS_$3}, \
		${call arch_object_list_template,$1,$2,$3,${arch}} \
	}
endef

define arch_object_list_template #(target, configuration, platform, arch)
	${foreach source,${SOURCES_$1} ${SOURCES_$1_$3}, \
		build/intermediate/$1-$2-$3-$4/${notdir ${basename ${source}}}.o \
	}
endef

#Produces OBJECTS_${target}_${configuration} variables for each permutation of target and configuration in that target
${foreach target,${TARGETS}, \
	${foreach configuration,${CONFIGURATIONS_${target}}, \
		${eval OBJECTS_${target}_${configuration} = ${call configuration_object_list_template,${target},${configuration}}} \
	} \
}



define create_directory_target_template #(dir)
.LOW_RESOLUTION_TIME: $1
$1:
	mkdir -p $1
endef

${foreach target,${TARGETS}, \
	${foreach configuration,${CONFIGURATIONS_${target}}, \
		${foreach platform,${PLATFORMS_${target}}, \
			${eval ${call create_directory_target_template,build/${target}/${configuration}-${platform}}} \
			${foreach arch,${ARCHS_${platform}}, \
				${eval ${call create_directory_target_template,build/intermediate/${target}-${configuration}-${platform}-${arch}}} \
			} \
		} \
	} \
}
${eval ${call create_directory_target_template,build/intermediate}}



define include_ccflags_template #(target, platform)
-I source -I include \
${foreach stem_dependency,${STEM_LIBRARY_DEPENDENCIES_$1} ${STEM_LIBRARY_DEPENDENCIES_$1_$2},-I lib/${dir ${stem_dependency}}include} \
${foreach thirdparty_dependency,${THIRDPARTY_LIBRARY_DEPENDENCIES_$1} ${THIRDPARTY_LIBRARY_DEPENDENCIES_$1_$2},-I lib/${dir ${thirdparty_dependency}}include} \
${foreach source_dependency,${STEM_SOURCE_DEPENDENCIES_$1} ${STEM_SOURCE_DEPENDENCIES_$1_$2},-I dep/${word 1,${subst /, ,${source_dependency}}}/source}
endef

define define_ccflags_template #(target, configuration, platform, arch)
-DSTEM_TARGET_$1 -DSTEM_CONFIGURATION_$2 -DSTEM_PLATFORM_$3 -DSTEM_ARCH_$4
endef

define dependency_template #(target, configuration, platform, arch, source_file)
build/intermediate/$1-$2-$3-$4/${notdir ${basename $5}}.d: $5 ${PREREQS_$1} | build/intermediate/$1-$2-$3-$4
	${CC_$3_$4} ${CCFLAGS} ${CCFLAGS_$1} ${CCFLAGS_$2} ${CCFLAGS_$3} ${call include_ccflags_template,$1,$3} ${call define_ccflags_template,$1,$2,$3,$4} -MM -o $$@.temp $5
	sed 's,\(${notdir ${basename $5}}\)\.o[ :]*,$${basename $$@}.o $${basename $$@}.d: ,g' < $$@.temp > $$@
endef

#Produces dependency build targets for all source files in each configuration/platform/arch
ifeq ($(filter clean full_dist commit_dist analyze analyze_clang analyze_splint,${MAKECMDGOALS}),)
${foreach target,${TARGETS}, \
	${foreach configuration,${CONFIGURATIONS_${target}}, \
		${foreach platform,${PLATFORMS_${target}}, \
			${foreach arch,${ARCHS_${platform}}, \
				${foreach source,${SOURCES_${target}} ${SOURCES_${target}_${platform}}, \
					${eval ${call dependency_template,${target},${configuration},${platform},${arch},${source}}} \
					${eval -include build/intermediate/${target}-${configuration}-${platform}-${arch}/${notdir ${basename ${source}}}.d} \
				} \
			} \
		} \
	} \
}
endif



define compile_template #(target, configuration, platform, arch, source_file)
build/intermediate/$1-$2-$3-$4/${notdir ${basename $5}}.o: $5 ${PREREQS_$1} | build/intermediate/$1-$2-$3-$4
	${CC_$3_$4} ${CCFLAGS} ${CCFLAGS_$1} ${CCFLAGS_$2} ${CCFLAGS_$3} ${call include_ccflags_template,$1,$3} ${call define_ccflags_template,$1,$2,$3,$4} -c -o $$@ $5
endef

#Produces object build targets for all source files in each configuration/platform/arch
${foreach target,${TARGETS}, \
	${foreach configuration,${CONFIGURATIONS_${target}}, \
		${foreach platform,${PLATFORMS_${target}}, \
			${foreach arch,${ARCHS_${platform}}, \
				${foreach source,${SOURCES_${target}} ${SOURCES_${target}_${platform}}, \
					${eval ${call compile_template,${target},${configuration},${platform},${arch},${source}}} \
				} \
			} \
		} \
	} \
}



define library_template #(target, configuration, platform, arch, output_file)
build/intermediate/$1-$2-$3-$4/$5: ${call arch_object_list_template,$1,$2,$3,$4}
	${AR_$3} rc $$@ $$^
	${RANLIB_$3} $$@
endef

#Produces static library build targets for each arch/platform/target for library targets
${foreach target,${LIBRARY_TARGETS}, \
	${foreach configuration,${CONFIGURATIONS_${target}}, \
		${foreach platform,${PLATFORMS_${target}}, \
			${foreach arch,${ARCHS_${platform}}, \
				${eval ${call library_template,${target},${configuration},${platform},${arch},${TARGET_NAME_${target}}.a}} \
			} \
		} \
	} \
}



define executable_template #(target, configuration, platform, arch, output_file, dependent_libraries)
build/intermediate/$1-$2-$3-$4/$5: ${call arch_object_list_template,$1,$2,$3,$4} $6
	${CC_$3_$4} -o $$@ $$^ ${LINKFLAGS} ${LINKFLAGS_$3}
endef

define library_dependency_template #(target, configuration, platform)
	${foreach link_library,${LINK_ORDER}, \
		${foreach library,${filter ${link_library},${PROJECT_LIBRARY_DEPENDENCIES_$1} ${PROJECT_LIBRARY_DEPENDENCIES_$1_$3}}, \
			build/${library}/$2-$3/${TARGET_NAME_${library}}.a \
		} \
		${foreach library,${filter ${link_library},${STEM_LIBRARY_DEPENDENCIES_$1} ${STEM_LIBRARY_DEPENDENCIES_$1_$3}}, \
			lib/${dir ${library}}library/$2-$3/${notdir ${library}} \
		} \
		${foreach library,${filter ${link_library},${STEM_SOURCE_DEPENDENCIES_$1} ${STEM_SOURCE_DEPENDENCIES_$1_$3}}, \
			dep/${word 1,${subst /, ,${library}}}/build/${word 2,${subst /, ,${library}}}/$2-$3/${word 3,${subst /, ,${library}}} \
		} \
		${foreach library,${filter ${link_library},${THIRDPARTY_LIBRARY_DEPENDENCIES_$1} ${THIRDPARTY_LIBRARY_DEPENDENCIES_$1_$3}}, \
			lib/${dir ${library}}library/$3/${notdir ${library}} \
		} \
	}
endef

#Produces executable build targets for each arch/platform/target for executable and application targets
${foreach target,${EXECUTABLE_TARGETS} ${APPLICATION_TARGETS}, \
	${foreach configuration,${CONFIGURATIONS_${target}}, \
		${foreach platform,${PLATFORMS_${target}}, \
			${foreach arch,${ARCHS_${platform}}, \
				${eval ${call executable_template,${target},${configuration},${platform},${arch},${TARGET_NAME_${target}}${EXECUTABLE_SUFFIX_${platform}},${call library_dependency_template,${target},${configuration},${platform}}}} \
			} \
		} \
	} \
}



define dependency_submake_template #(dependency)
.PHONY: $1
$1:
	${MAKE} -C dep/${word 1,${subst /, ,$1}}
endef

#Invokes make for each source dependency
${foreach dependency,${sort ${foreach target,${TARGETS},${foreach platform,${PLATFORMS_${target}},${STEM_SOURCE_DEPENDENCIES_${target}} ${STEM_SOURCE_DEPENDENCIES_${target}_${platform}}}}}, \
	${eval ${call dependency_submake_template,${dependency}}} \
}



define thin_binary_list_template #(target, configuration, platform, target_name)
	${foreach arch,${ARCHS_$3}, \
		build/intermediate/$1-$2-$3-${arch}/$4 \
	}
endef

#Produces THIN_BINARIES_${target}_${configuration}_${platform} variables for each target/configuration/platform for library targets
${foreach target,${LIBRARY_TARGETS}, \
	${foreach configuration,${CONFIGURATIONS_${target}}, \
		${foreach platform,${PLATFORMS_${target}}, \
			${eval THIN_BINARIES_${target}_${configuration}_${platform} = ${call thin_binary_list_template,${target},${configuration},${platform},${TARGET_NAME_${target}}.a}} \
		} \
	} \
}

#Produces THIN_BINARIES_${target}_${configuration}_${platform} variables for each target/configuration/platform for executable targets
${foreach target,${EXECUTABLE_TARGETS}, \
	${foreach configuration,${CONFIGURATIONS_${target}}, \
		${foreach platform,${PLATFORMS_${target}}, \
			${eval THIN_BINARIES_${target}_${configuration}_${platform} = ${call thin_binary_list_template,${target},${configuration},${platform},${TARGET_NAME_${target}}${EXECUTABLE_SUFFIX_${platform}}}} \
		} \
	} \
}

#Produces THIN_BINARIES_${target}_${configuration}_${platform} variables for each target/configuration/platform for application targets
${foreach target,${APPLICATION_TARGETS}, \
	${foreach configuration,${CONFIGURATIONS_${target}}, \
		${foreach platform,${PLATFORMS_${target}}, \
			${eval THIN_BINARIES_${target}_${configuration}_${platform} = ${call thin_binary_list_template,${target},${configuration},${platform},${TARGET_NAME_${target}}${EXECUTABLE_SUFFIX_${platform}}}} \
		} \
	} \
}



define assemble_library_macosx #(target, configuration, platform)
build/$1/$2-$3/${TARGET_NAME_$1}.a: ${THIN_BINARIES_$1_$2_$3} | build/$1/$2-$3
	lipo -create -output $$@ ${THIN_BINARIES_$1_$2_$3}
endef

define assemble_library_linux #(target, configuration, platform)
build/$1/$2-$3/${TARGET_NAME_$1}.a: ${THIN_BINARIES_$1_$2_$3} | build/$1/$2-$3
	cp ${THIN_BINARIES_$1_$2_$3} $$@
endef

define assemble_library_windows #(target, configuration, platform)
build/$1/$2-$3/${TARGET_NAME_$1}.a: ${THIN_BINARIES_$1_$2_$3} | build/$1/$2-$3
	cp ${THIN_BINARIES_$1_$2_$3} $$@
endef

#Produces final library build targets
${foreach target,${LIBRARY_TARGETS}, \
	${foreach configuration,${CONFIGURATIONS_${target}}, \
		${foreach platform,${PLATFORMS_${target}}, \
			${eval ${call assemble_library_${HOST_PLATFORM},${target},${configuration},${platform}}} \
		} \
	} \
}

define copy_target_resources #(target, platform, resources_dir)
	${foreach resource,${RESOURCES_$1} ${RESOURCES_$1_$2}, \
		cp -r ${resource} $3${newline_and_tab} \
	}
	find $3 -name .svn -print0 | xargs -0 rm -rf
endef

define assemble_executable_macosx #(target, configuration, platform)
build/$1/$2-$3/${TARGET_NAME_$1}: ${THIN_BINARIES_$1_$2_$3} ${RESOURCES_$1} ${RESOURCES_$1_$3} | build/$1/$2-$3
	lipo -create -output $$@ ${THIN_BINARIES_$1_$2_$3}
	${call copy_target_resources,$1,$3,$${dir $$@}}
endef

define assemble_executable_linux #(target, configuration, platform)
build/$1/$2-$3/${TARGET_NAME_$1}: ${THIN_BINARIES_$1_$2_$3} ${RESOURCES_$1} ${RESOURCES_$1_$3} | build/$1/$2-$3
	cp ${THIN_BINARIES_$1_$2_$3} $$@
	${call copy_target_resources,$1,$3,$${dir $$@}}
endef

define assemble_executable_windows #(target, configuration, platform)
build/$1/$2-$3/${TARGET_NAME_$1}.exe: ${THIN_BINARIES_$1_$2_$3} ${RESOURCES_$1} ${RESOURCES_$1_$3} | build/$1/$2-$3
	cp ${THIN_BINARIES_$1_$2_$3} $$@
	${call copy_target_resources,$1,$3,$${dir $$@}}
endef

#Produces final executable build targets
${foreach target,${EXECUTABLE_TARGETS}, \
	${foreach configuration,${CONFIGURATIONS_${target}}, \
		${foreach platform,${PLATFORMS_${target}}, \
			${eval ${call assemble_executable_${HOST_PLATFORM},${target},${configuration},${platform}}} \
		} \
	} \
}

PLIST_FILE_testharness_macosx = test_resources/Info_testharness_macosx.plist

PLIST_FILE_testharness_iphonesimulator = test_resources/Info_testharness_iphone.plist
PLIST_PLATFORM_CASED_iphonesimulator = iPhoneSimulator
PLIST_PLATFORM_LOWER_iphonesimulator = iphonesimulator
PLIST_SDK_NAME_iphonesimulator = iphonesimulator${IPHONE_BUILD_SDK_VERSION}

PLIST_FILE_testharness_iphoneos = test_resources/Info_testharness_iphone.plist
PLIST_PLATFORM_CASED_iphoneos = iPhoneOS
PLIST_PLATFORM_LOWER_iphoneos = iphoneos
PLIST_SDK_NAME_iphoneos = iphoneos${IPHONE_BUILD_SDK_VERSION}

define create_app_bundle #(target, platform, executable_dir, plist_dir, resources_dir)
	mkdir -p $3 $4 $5
	sed -e "s/\$$$${PRODUCT_NAME}/${TARGET_NAME_$1}/g" \
	    -e "s/\$$$${HUMAN_READABLE_PRODUCT_NAME}/${HUMAN_READABLE_TARGET_NAME_$1}/g" \
	    -e "s/\$$$${VERSION}/${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_TWEAK}/g" \
	    -e "s/\$$$${COPYRIGHT_YEAR}/"`date +%Y`"/g" \
	    -e "s/\$$$${BUILD_NUMBER}/0/g" \
	    -e "s/\$$$${PLATFORM_CASED}/${PLIST_PLATFORM_CASED_$2}/g" \
	    -e "s/\$$$${PLATFORM_LOWER}/${PLIST_PLATFORM_LOWER_$2}/g" \
	    -e "s/\$$$${SDK}/${PLIST_SDK_NAME_$2}/g" \
	    ${PLIST_FILE_$1_$2} > $4/Info.plist
	echo "APPL????" > $4/PkgInfo
	${call copy_target_resources,$1,$2,$5}
endef

define assemble_application_macosx #(target, configuration)
build/$1/$2-macosx/${TARGET_NAME_$1}.app/Contents/MacOS/${TARGET_NAME_$1}: ${THIN_BINARIES_$1_$2_macosx} ${RESOURCES_$1} ${RESOURCES_$1_macosx} | build/$1/$2-macosx
	${call create_app_bundle,$1,macosx,$${dir $$@},build/$1/$2-macosx/${TARGET_NAME_$1}.app/Contents,build/$1/$2-macosx/${TARGET_NAME_$1}.app/Contents/Resources}
	lipo -create -output $$@ ${THIN_BINARIES_$1_$2_macosx}
endef

define assemble_application_iphonesimulator #(target, configuration)
build/$1/$2-iphonesimulator/${TARGET_NAME_$1}.app/${TARGET_NAME_$1}: ${THIN_BINARIES_$1_$2_iphonesimulator} ${RESOURCES_$1} ${RESOURCES_$1_iphonesimulator} | build/$1/$2-iphonesimulator
	${call create_app_bundle,$1,iphonesimulator,$${dir $$@},$${dir $$@},$${dir $$@}}
	lipo -create -output $$@ ${THIN_BINARIES_$1_$2_iphonesimulator}
endef

define assemble_application_iphoneos #(target, configuration)
build/$1/$2-iphoneos/${TARGET_NAME_$1}.app/${TARGET_NAME_$1}: ${THIN_BINARIES_$1_$2_iphoneos} ${RESOURCES_$1} ${RESOURCES_$1_iphoneos} | build/$1/$2-iphoneos
	${call create_app_bundle,$1,iphoneos,$${dir $$@},$${dir $$@},$${dir $$@}}
	lipo -create -output $$@ ${THIN_BINARIES_$1_$2_iphoneos}
endef

define assemble_application_linux #(target, configuration)
build/$1/$2-linux/${TARGET_NAME_$1}: ${THIN_BINARIES_$1_$2_linux} ${RESOURCES_$1} ${RESOURCES_$1_linux} | build/$1/$2-linux
	mkdir -p build/$1/$2-linux/Resources
	${call copy_target_resources,$1,linux,build/$1/$2-linux/Resources}
	cp ${THIN_BINARIES_$1_$2_linux} $$@
endef

define assemble_application_windows #(target, configuration)
build/$1/$2-windows/${TARGET_NAME_$1}.exe: ${THIN_BINARIES_$1_$2_windows} ${RESOURCES_$1} ${RESOURCES_$1_windows} | build/$1/$2-windows
	mkdir -p build/$1/$2-windows/Resources
	${call copy_target_resources,$1,windows,build/$1/$2-windows/Resources}
	cp ${THIN_BINARIES_$1_$2_windows} $$@
endef

#Produces final application build targets
${foreach target,${APPLICATION_TARGETS}, \
	${foreach configuration,${CONFIGURATIONS_${target}}, \
		${foreach platform,${PLATFORMS_${target}}, \
			${eval ${call assemble_application_${platform},${target},${configuration}}} \
		} \
	} \
}

define library_template #(target, configuration, platform)
${STEM_SOURCE_DEPENDENCIES_$1} ${STEM_SOURCE_DEPENDENCIES_$1_$3} build/$1/$2-$3/${TARGET_NAME_$1}.a
endef

define executable_template #(target, configuration, platform)
${STEM_SOURCE_DEPENDENCIES_$1} ${STEM_SOURCE_DEPENDENCIES_$1_$3} build/$1/$2-$3/${TARGET_NAME_$1}${EXECUTABLE_SUFFIX_$3}
endef

define application_template #(target, configuration, platform)
${STEM_SOURCE_DEPENDENCIES_$1} ${STEM_SOURCE_DEPENDENCIES_$1_$3} build/$1/$2-$3/${call application_file_template_$3,$1}
endef

define application_file_template_macosx #(target)
${TARGET_NAME_$1}.app/Contents/MacOS/${TARGET_NAME_$1}
endef

define application_file_template_iphonesimulator #(target)
${TARGET_NAME_$1}.app/${TARGET_NAME_$1}
endef

define application_file_template_iphoneos #(target)
${TARGET_NAME_$1}.app/${TARGET_NAME_$1}
endef

define application_file_template_linux #(target)
${TARGET_NAME_$1}
endef

define application_file_template_windows #(target)
${TARGET_NAME_$1}.exe
endef

define target_template #(target, target_type)
.PHONY: $1
$1: ${foreach configuration,${CONFIGURATIONS_$1},${foreach platform,${PLATFORMS_$1},${call $2_template,$1,${configuration},${platform}}}}
endef

${foreach target,${LIBRARY_TARGETS}, \
	${eval ${call target_template,${target},library}} \
}

${foreach target,${EXECUTABLE_TARGETS}, \
	${eval ${call target_template,${target},executable}} \
}

${foreach target,${APPLICATION_TARGETS}, \
	${eval ${call target_template,${target},application}} \
}

.PHONY: test
test: ${foreach platform,${PLATFORMS_unittest},run_unittests_${platform}}

.PHONY: run_unittests_macosx
run_unittests_macosx: unittest
	./build/unittest/debug-macosx/${TARGET_NAME_unittest} "${CURDIR}/build/unittest/debug-macosx"

.PHONY: run_unittests_iphonesimulator
run_unittests_iphonesimulator: unittest
	DYLD_ROOT_PATH=${SDKROOT_iphonesimulator} \
	./build/unittest/debug-iphonesimulator/${TARGET_NAME_unittest} "${CURDIR}/build/unittest/debug-iphonesimulator"

.PHONY: run_unittests_linux
run_unittests_linux: unittest
	./build/unittest/debug-linux/${TARGET_NAME_unittest} "${CURDIR}/build/unittest/debug-linux"

.PHONY: run_unittests_windows
run_unittests_windows: unittest
	./build/unittest/debug-windows/${TARGET_NAME_unittest}.exe "${CURDIR}/build/unittest/debug-windows"

define analyze_file_template_clang #(target, platform, file)
build/analyzer-results/clang-$1-$2/${basename ${notdir $3}}.txt: $3 ${PREREQS_$1} | build/analyzer-results/clang-$1-$2
	${CLANG_$2} --analyze ${call include_ccflags_template,$1,$2} ${call define_ccflags_template,$1,analyze,$2,none} ${CLANGFLAGS} ${CLANGFLAGS_$1} ${CLANGFLAGS_$2} -o $${basename $$@}.plist $3 > $$@ 2>&1; true
	@cat $$@
endef

define analyze_file_template_splint #(target, platform, file)
build/analyzer-results/splint-$1-$2/${basename ${notdir $3}}.txt: $3 ${PREREQS_$1} | build/analyzer-results/splint-$1-$2
	${SPLINT_$2} ${call include_ccflags_template,$1,$2} ${call define_ccflags_template,$1,analyze,$2,none} ${SPLINTFLAGS} ${SPLINTFLAGS_$1} ${SPLINTFLAGS_$2} $3 > $$@ 2>&1; true
	@cat $$@
endef

define analyzed_sources_template #(analyzer, target, platform)
	${sort ${filter-out ${ANALYZER_EXCLUDE_SOURCES_$1},${SOURCES_$2} ${SOURCES_$2_$3}}}
endef

define analyzer_output_template #(analyzer, target, platform)
	${foreach file,${call analyzed_sources_template,$1,$2,$3}, \
		build/analyzer-results/$1-$2-$3/${basename ${notdir ${file}}}.txt \
	}
endef

define analyze_target_template #(analyzer, target, platform)
.PHONY: analyze_$1_$2_$3
analyze_$1_$2_$3: ${call analyzer_output_template,$1,$2,$3}
endef

define analyze_template #(analyzer)
.PHONY: analyze_$1
analyze_$1: ${foreach target,${TARGETS},${foreach platform,${PLATFORMS_${target}},analyze_$1_${target}_${platform}}}
endef

${foreach analyzer,${ANALYZERS}, \
	${eval ${call analyze_template,${analyzer}}} \
	${foreach target,${TARGETS}, \
		${foreach platform,${PLATFORMS_${target}}, \
			${eval ${call analyze_target_template,${analyzer},${target},${platform}}} \
			${foreach file,${call analyzed_sources_template,${analyzer},${target},${platform}}, \
				${eval ${call analyze_file_template_${analyzer},${target},${platform},${file}}} \
			} \
		} \
	} \
}

${foreach analyzer,${ANALYZERS}, \
	${foreach target,${TARGETS}, \
		${foreach platform,${PLATFORMS_${target}}, \
			${eval ${call create_directory_target_template,build/analyzer-results/${analyzer}-${target}-${platform}}} \
		} \
	} \
}

.PHONY: analyze
analyze: ${foreach analyzer,${ANALYZERS},analyze_${analyzer}}

${foreach dir,${sort ${foreach include_file,${INCLUDES},build/include/${notdir ${patsubst %/,%,${dir ${include_file}}}}}}, \
	${eval ${call create_directory_target_template,${dir}}} \
}

.PHONY: include
include: ${INCLUDES} | ${foreach include_file,${INCLUDES},build/include/${notdir ${patsubst %/,%,${dir ${include_file}}}}}
	${foreach include_file,${INCLUDES}, \
		cp ${include_file} build/include/${notdir ${patsubst %/,%,${dir ${include_file}}}}${newline_and_tab} \
	}

.PHONY: clean
clean:
	rm -rf build
	rm -rf dist #master_source_only
	${foreach dependency,${sort ${foreach target,${TARGETS},${foreach platform,${PLATFORMS_${target}},${STEM_SOURCE_DEPENDENCIES_${target}} ${STEM_SOURCE_DEPENDENCIES_${target}_${platform}}}}}, \
		${MAKE} -C dep/${word 1,${subst /, ,${dependency}}} clean${newline_and_tab} \
	}

TARGET_SUFFIX_ipad = _ipad
TARGET_SUFFIX_iphone4 = _iphone4
IPHONE_SDK_VERSION_iphone ?= 4.2
IPHONE_SDK_VERSION_ipad ?= 3.2
IPHONE_SDK_VERSION_iphone4 ?= 4.2
IPHONESIMULATOR_APPLICATIONS_DIR_iphone ?= ${HOME}/Library/Application Support/iPhone Simulator/4.2/Applications
IPHONESIMULATOR_APPLICATIONS_DIR_ipad ?= ${HOME}/Library/Application Support/iPhone Simulator/${IPHONE_SDK_VERSION_ipad}/Applications
IPHONESIMULATOR_APPLICATIONS_DIR_iphone4 ?= ${HOME}/Library/Application Support/iPhone Simulator/4.2/Applications
SIMULATE_DEVICE_iphone = iPhone
SIMULATE_DEVICE_ipad = iPad
SIMULATE_DEVICE_iphone4 = iPhone 4
SIMULATE_SDKROOT_iphone = /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator${IPHONE_SDK_VERSION_iphone}.sdk
SIMULATE_SDKROOT_ipad = /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator${IPHONE_SDK_VERSION_ipad}.sdk
SIMULATE_SDKROOT_iphone4 = /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator${IPHONE_SDK_VERSION_iphone4}.sdk

define install_target_iphonesimulator_template #(target, simulate_device)
.PHONY: install_$1_iphonesimulator${TARGET_SUFFIX_$2}
install_$1_iphonesimulator${TARGET_SUFFIX_$2}: $1
	killall "iPhone Simulator"; true
	rm -rf "${IPHONESIMULATOR_APPLICATIONS_DIR_$2}/${TARGET_NAME_$1}"
	mkdir -p "${IPHONESIMULATOR_APPLICATIONS_DIR_$2}/${TARGET_NAME_$1}/Documents"
	mkdir -p "${IPHONESIMULATOR_APPLICATIONS_DIR_$2}/${TARGET_NAME_$1}/Library/Preferences"
	mkdir -p "${IPHONESIMULATOR_APPLICATIONS_DIR_$2}/${TARGET_NAME_$1}/tmp"
	cp -r "build/$1/debug-iphonesimulator/${TARGET_NAME_$1}.app" "${IPHONESIMULATOR_APPLICATIONS_DIR_$2}/${TARGET_NAME_$1}"
	defaults write com.apple.iphonesimulator SimulateDevice -string "${SIMULATE_DEVICE_$2}"
	defaults write com.apple.iphonesimulator SimulateSDKRoot -string "${SIMULATE_SDKROOT_$2}"
	defaults write com.apple.iphonesimulator currentSDKRoot -string "${SIMULATE_SDKROOT_$2}"
	open "/Developer/Platforms/iPhoneSimulator.platform/Developer/Applications/iPhone Simulator.app"
endef

define add_blob_header #(source_file, target_file)
	ruby -e "contents = \"\"; File.open(\"$1\", \"r\") {|file| contents = file.read}; File.open(\"$2\", \"w\") {|file| file.write(\"\xFA\xDE\x71\x71\"); file.write([contents.length + 8].pack(\"N\")); file.write(contents)}"
endef

RESOURCE_RULES_PLIST = /Developer/Platforms/MacOSX.platform/ResourceRules.plist

define codesign_target_iphoneos_template #(target)
.PHONY: codesign_$1_iphoneos
codesign_$1_iphoneos: $1
	sed -e "s/\$$$${PRODUCT_NAME}/${TARGET_NAME_$1}/g" test_resources/Entitlements.plist > build/intermediate/Entitlements.plist
	${call add_blob_header,build/intermediate/Entitlements.plist,build/intermediate/Entitlements.xcent}
	export CODESIGN_ALLOCATE=/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/codesign_allocate
	${foreach configuration,${CONFIGURATIONS_$1},\
		cp "${RESOURCE_RULES_PLIST}" "build/$1/${configuration}-iphoneos/${TARGET_NAME_$1}.app"${newline_and_tab} \
		/usr/bin/codesign -f -s ${CODESIGN_IDENTITY} --resource-rules=${RESOURCE_RULES_PLIST} --entitlements=build/intermediate/Entitlements.xcent "build/$1/${configuration}-iphoneos/${TARGET_NAME_$1}.app"${newline_and_tab} \
	}
endef

${foreach target,${APPLICATION_TARGETS}, \
	${eval ${call install_target_iphonesimulator_template,${target},iphone}} \
	${eval ${call install_target_iphonesimulator_template,${target},ipad}} \
	${eval ${call install_target_iphonesimulator_template,${target},iphone4}} \
	${eval ${call codesign_target_iphoneos_template,${target}}} \
}

#master_source_only_begin
.PHONY: full_dist
full_dist: clean all
	mkdir dist dist/include dist/library dist/testharness
	cp Changes.txt License.txt ReadMe.txt dist
	cp -r build/include/* dist/include
	cp -r build/library/* dist/library
	cp -r build/testharness/* dist/testharness

.PHONY: append_dist
append_dist: all
	cp -r build/library/* dist/library
	cp -r build/testharness/* dist/testharness

.PHONY: commit_dist
commit_dist:
	svn import --no-ignore -m "Automated release of ${PROJECT_NAME} ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_TWEAK} from ${HOST_PLATFORM}" dist ${SVNROOT}/Releases/${PROJECT_NAME}/${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_TWEAK}

define filter_source_dist_file #(infile, outfile)
	ruby -e "print STDIN.read.gsub(/^.*#master_source_only\b.*$\n?/, '').gsub(/\n?#master_source_only_begin[\s\S]*?#master_source_only_end/, '').gsub(/#source_dist_only/, '')" < "$1" > "$2"
endef

.PHONY: source_dist
source_dist:
	mkdir -p build/source_dist/include
	mkdir -p build/source_dist/lib
	mkdir -p build/source_dist/source/gamepad
	mkdir -p build/source_dist/test_resources
	mkdir -p build/source_dist/test_source/testharness
	mkdir -p build/source_dist/test_source/unittest/framework
	mkdir -p build/source_dist/test_source/unittest/suites
	
	${call filter_source_dist_file,Makefile,build/source_dist/Makefile}
	cp Changes.txt License.txt ReadMe.txt version build/source_dist
	
	cp -r include/* build/source_dist/include
	cp -r lib/* build/source_dist/lib
	find build/source_dist/include -name .svn -print0 | xargs -0 rm -rf
	find build/source_dist/lib -name .svn -print0 | xargs -0 rm -rf
	
	cp source/gamepad/* build/source_dist/source/gamepad
	
	cp test_resources/* build/source_dist/test_resources
	
	cp test_source/testharness/* build/source_dist/test_source/testharness
	cp test_source/unittest/framework/* build/source_dist/test_source/unittest/framework
	cp test_source/unittest/suites/* build/source_dist/test_source/unittest/suites
#master_source_only_end
