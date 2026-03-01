
# ninja/parallel builds disable output colors, forcing them here.
# Needs to be at global scope to catch all targets.
# cmake will have this build into 3.24
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


# compiler cache
option(WANT_CCACHE "Use ccache to speed up rebuilds" OFF)
if(WANT_CCACHE)
	if(NOT FOUND_CCACHE)
		find_program(CCACHE_EXECUTABLE ccache)

		if(CCACHE_EXECUTABLE)
			message(STATUS "Using ccache as CXX compiler launcher")
			set(FOUND_CCACHE 1 CACHE INTERNAL "ccache has been found")
		endif()
	endif()

	if(CCACHE_EXECUTABLE)
		set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
	endif()
endif()

# global scope, since ASAN needs to catch all targets
option(WANT_ASAN "build with address sanitizer" OFF)
if(WANT_ASAN)
	message(STATUS "Building with address sanitizer")
	add_compile_options(-fno-omit-frame-pointer -fsanitize=address)
	add_link_options(-fno-omit-frame-pointer -fsanitize=address)
endif()

# formatting and static analysis
file(GLOB_RECURSE ALL_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

if(NOT FOUND_ASTYLE)
	find_program(ASTYLE_EXECUTABLE astyle)

	if(ASTYLE_EXECUTABLE)
		message(STATUS "Found source code formatter: astyle")
		set(FOUND_ASTYLE 1 CACHE INTERNAL "Astyle has been found")
	endif()
endif()
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
endif()

if(NOT FOUND_CPPCHECK)
	find_program(CPPCHECK_EXECUTABLE cppcheck)
	if(CPPCHECK_EXECUTABLE)
		message(STATUS "Found static analysis tool: cppcheck")
		set(FOUND_CPPCHECK 1 CACHE INTERNAL "Cppcheck has been found")
	endif()
endif()
if(CPPCHECK_EXECUTABLE)
	list(APPEND CPPCHECK_ARGS
		--enable=warning,style,performance,portability,unusedFunction
		--std=c++11
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
endif()

if(NOT FOUND_ASTYLE AND NOT FOUND_CPPCHECK)
	message(STATUS "No source code formatter or static analysis tool enabled.")
endif()
