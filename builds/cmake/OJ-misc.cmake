
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

# global scope, since ASAN needs to catch all targets
option(WANT_ASAN "build with adress sanitizer" OFF)
if(WANT_ASAN)
	add_compile_options(-fno-omit-frame-pointer -fsanitize=address)
	add_link_options(-fno-omit-frame-pointer -fsanitize=address)
endif()
