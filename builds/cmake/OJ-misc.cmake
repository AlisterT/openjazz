
# ninja/parallel builds disable output colors, forcing them here.
# Needs to be at global scope to catch all targets.
# cmake has this build-in since 3.24
if(CMAKE_VERSION VERSION_LESS "3.24")
	option(CMAKE_COLOR_DIAGNOSTICS "Always produce ANSI-colored output" OFF)
	if(CMAKE_COLOR_DIAGNOSTICS)
		if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
			add_compile_options("-fdiagnostics-color=always")
		elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
			add_compile_options("-fcolor-diagnostics")
		endif()
	endif()
endif()

# build type: allow user override and multi-configuration generators
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	# Set build type if none was specified, git checkouts default to debug
	set(default_build_type "Release")
	if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
		set(default_build_type "Debug")
	endif()
	message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
	set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# compiler cache
set(CCACHE_STATUS "Disabled")
option(WANT_CCACHE "Use ccache to speed up rebuilds" OFF)
if(WANT_CCACHE)
	find_program(CCACHE_EXECUTABLE ccache)
	if(CCACHE_EXECUTABLE)
		set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
		set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
		set(CCACHE_STATUS "Enabled, ccache")
	else()
		set(CCACHE_STATUS "Not found")
	endif()
endif()

# global scope, since ASAN needs to catch all targets
set(ASAN_STATUS "Disabled")
option(WANT_ASAN "build with address sanitizer" OFF)
if(WANT_ASAN)
	add_compile_options(-fno-omit-frame-pointer -fsanitize=address)
	add_link_options(-fno-omit-frame-pointer -fsanitize=address)
	set(ASAN_STATUS "Enabled")
endif()

# formatting and static analysis
file(GLOB_RECURSE ALL_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

set(ASTYLE_STATUS "Not available")
find_program(ASTYLE_EXECUTABLE astyle)
if(ASTYLE_EXECUTABLE)
	list(APPEND ASTYLE_ARGS
		--suffix=none
		--style=attach
		--indent=tab=4
		--pad-oper
		--pad-header
		--unpad-paren
		--max-code-length=100
		--break-after-logical
		--attach-closing-while
		--align-pointer=type
		--align-reference=name
		--indent-classes
		--indent-preproc-block
		--indent-switches
		--min-conditional-indent=0)

	add_custom_target(format
		COMMAND ${ASTYLE_EXECUTABLE} ${ASTYLE_ARGS} ${ALL_SRC}
		COMMENT "Running astyle to format source code"
		VERBATIM)
	set(ASTYLE_STATUS "Enabled, astyle (target 'format')")
endif()

set(CPPCHECK_STATUS "Not available")
find_program(CPPCHECK_EXECUTABLE cppcheck)
if(CPPCHECK_EXECUTABLE)
	list(APPEND CPPCHECK_ARGS
		--enable=warning,style,performance,portability,unusedFunction
		--std=c++14
		--language=c++
		-I${CMAKE_CURRENT_SOURCE_DIR}/src
		-U__SYMBIAN32__ -UUIQ3 -UENABLE_JJ2                     # unmaintained
		-UGP2X -UWIZ -UDINGOO -UCAANOO -UGAMESHELL -U__riscos__ # contributed
		-UPSP -U__vita__ -U__3DS__ -U__wii__ -U__SWITCH__       # homebrew
		#--enable=information
	)

	add_custom_target(cppcheck
		COMMAND ${CPPCHECK_EXECUTABLE} ${CPPCHECK_ARGS} ${ALL_SRC}
		COMMENT "Running cppcheck for static analysis"
		USES_TERMINAL VERBATIM)
	set(CPPCHECK_STATUS "Enabled, cppcheck (target 'cppcheck')")
endif()
