
include(CheckCXXSourceCompiles)

# This module checks if the C++ compiler supports the restrict keyword or
# some variant of it:
# - restrict                    (The standard C99 keyword, not in C++ standard, Windows VS has it)
# - __restrict and __restrict__ (G++ has them)
# - _Restrict                   (seems to be used by Sun's compiler)
# Some C++ compilers do not support any variant, in which case the RESTRICT_KEYWORD variable is empty

if(FOUND_RESTRICT_KEYWORD)
	return() # already tested
endif()

set(_MSG "Looking for restrict keyword")
message(STATUS ${_MSG})

set(Find_restrict_KEYWORD_SRC "
char f(const char * PLACEHOLDER_RESTRICT_KEYWORD x){ return *x; }
int main(int argc, char *argv[]) { return 0; }
")

set(CMAKE_REQUIRED_QUIET TRUE) # silence tests
set(RESTRICT_KEYWORD) # empty
set(Find_restrict_KEYWORDS restrict __restrict __restrict__ _Restrict) # candidates

foreach(restrict_KEYWORD IN LISTS Find_restrict_KEYWORDS)
		string(REPLACE "PLACEHOLDER_RESTRICT_KEYWORD" "${restrict_KEYWORD}"
			_SRC "${Find_restrict_KEYWORD_SRC}")
		check_cxx_source_compiles("${_SRC}" HAVE_KEYWORD_${restrict_KEYWORD})
		if(HAVE_KEYWORD_${restrict_KEYWORD})
			set(RESTRICT_KEYWORD ${restrict_KEYWORD})
			break() # end loop
		endif()
endforeach()

if(RESTRICT_KEYWORD)
	message(STATUS "${_MSG} - found \"${RESTRICT_KEYWORD}\"")
	set(FOUND_RESTRICT_KEYWORD 1 CACHE INTERNAL "Restrict keyword has been found")
else()
	message(STATUS "${_MSG} - not found")
endif()
