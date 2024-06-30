#.rst:
# FindConfuse
# -----------
#
# Find the confuse Library
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``CONFUSE::libconfuse``
#   The ``confuse`` library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``CONFUSE_INCLUDE_DIRS``
#   where to find confuse headers.
# ``CONFUSE_LIBRARIES``
#   the libraries to link against to use confuse.
# ``CONFUSE_FOUND``
#   true if the confuse headers and libraries were found.

# pre-check
find_package(PkgConfig QUIET)
pkg_check_modules(PC_confuse QUIET confuse)
set(CONFUSE_VERSION ${PC_confuse_VERSION})

# check header
find_path(CONFUSE_INCLUDE_DIR
	NAMES confuse.h
	HINTS ${CONFUSE_ROOT} ${PC_confuse_INCLUDE_DIRS})

# check library
find_library(CONFUSE_LIBRARY
	NAMES confuse
	HINTS ${CONFUSE_ROOT} ${PC_confuse_LIBRARY_DIRS})

# check sanity
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(confuse
	REQUIRED_VARS CONFUSE_LIBRARY CONFUSE_INCLUDE_DIR
	VERSION_VAR CONFUSE_VERSION)

# set variables
mark_as_advanced(CONFUSE_INCLUDE_DIR CONFUSE_LIBRARY)
if(CONFUSE_FOUND)
	set(CONFUSE_INCLUDE_DIRS ${CONFUSE_INCLUDE_DIR})
	set(CONFUSE_LIBRARIES ${CONFUSE_LIBRARY})

	# add target
	if(NOT TARGET CONFUSE::libconfuse)
		add_library(CONFUSE::libconfuse UNKNOWN IMPORTED)
		set_target_properties(CONFUSE::libconfuse PROPERTIES
			IMPORTED_LOCATION "${CONFUSE_LIBRARY}"
			INTERFACE_INCLUDE_DIRECTORIES "${CONFUSE_INCLUDE_DIR}")
	endif()
endif()
