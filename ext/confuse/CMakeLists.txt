
# confuse

enable_language(C)

add_library(confuse STATIC
	confuse.c
	confuse.h
	lexer.c
)
target_include_directories(confuse
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)
target_compile_definitions(confuse PRIVATE
	PACKAGE_VERSION="3.3"
	PACKAGE_STRING="confuse 3.3"
	HAVE_CONFIG_H
	BUILDING_STATIC
)

include(CheckIncludeFile)
check_include_file(string.h HAVE_STRING_H)
check_include_file(strings.h HAVE_STRINGS_H)
check_include_file(sys/stat.h HAVE_SYS_STAT_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
if(WIN32)
	check_include_file(windows.h HAVE_WINDOWS_H)
else()
	check_include_file(unistd.h HAVE_UNISTD_H)
endif()

include(CheckFunctionExists)
check_function_exists(strdup HAVE_STRDUP)
check_function_exists(strndup HAVE_STRNDUP)
check_function_exists(strcasecmp HAVE_STRCASECMP)

check_function_exists(reallocarray HAVE_REALLOCARRAY)
if(NOT HAVE_REALLOCARRAY)
	target_sources(confuse PRIVATE reallocarray.c)
endif()
check_function_exists(fmemopen HAVE_FMEMOPEN)
if(NOT HAVE_FMEMOPEN)
	check_function_exists(funopen HAVE_FUNOPEN)
	target_sources(confuse PRIVATE fmemopen.c)
endif()

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
