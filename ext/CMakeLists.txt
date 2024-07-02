
# argparse

add_library(argparse STATIC argparse/argparse.h argparse/argparse.cpp)
target_include_directories(argparse PUBLIC argparse)
target_link_libraries(OpenJazz argparse)

# confuse

find_package(confuse)
if(NOT TARGET CONFUSE::libconfuse)
	add_subdirectory(confuse)
	add_library(CONFUSE::libconfuse ALIAS confuse)
endif()
target_link_libraries(OpenJazz CONFUSE::libconfuse)
target_compile_definitions(OpenJazz PRIVATE USE_CONFUSE)

# miniz

add_library(miniz STATIC miniz/miniz.h miniz/miniz.cpp)
target_compile_definitions(miniz PUBLIC MINIZ_NO_ARCHIVE_APIS)
#target_compile_definitions(miniz PRIVATE _LARGEFILE64_SOURCE)
target_include_directories(miniz PUBLIC miniz)
target_link_libraries(OpenJazz miniz)

# psmplug

add_library(psmplug STATIC
	psmplug/fastmix.cpp
	psmplug/load_psm.cpp
	psmplug/psmplug.cpp
	psmplug/psmplug.h
	psmplug/snd_dsp.cpp
	psmplug/snd_flt.cpp
	psmplug/snd_fx.cpp
	psmplug/sndfile.cpp
	psmplug/sndfile.h
	psmplug/sndmix.cpp
	psmplug/stdafx.h
	psmplug/tables.h
)
target_include_directories(psmplug PUBLIC psmplug)
target_link_libraries(OpenJazz psmplug)
