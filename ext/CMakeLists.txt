
# argparse

add_library(argparse STATIC argparse/argparse.h argparse/argparse.cpp)
target_include_directories(argparse INTERFACE argparse)

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
target_include_directories(psmplug INTERFACE psmplug)

# stb

add_library(stb STATIC stb/stb_rect_pack.h stb/stb_rect_pack.cpp)
target_include_directories(stb INTERFACE stb)

# miniz

add_library(miniz STATIC miniz/miniz.h miniz/miniz.cpp)
target_compile_definitions(miniz INTERFACE MINIZ_NO_ZLIB_COMPATIBLE_NAMES)
#target_compile_definitions(miniz PRIVATE _LARGEFILE64_SOURCE)
target_include_directories(miniz INTERFACE miniz)

# lodepng

add_library(lodepng STATIC
	lodepng/lodepng.h
	lodepng/lodepng.cpp
	lodepng/lodepng-miniz.h
	lodepng/lodepng-miniz.cpp
)
target_compile_definitions(lodepng INTERFACE
	LODEPNG_NO_COMPILE_ZLIB
	LODEPNG_NO_COMPILE_DISK
	LODEPNG_NO_COMPILE_CRC
	LODEPNG_NO_COMPILE_CPP
)
target_include_directories(lodepng INTERFACE lodepng)
target_link_libraries(lodepng PRIVATE miniz)

# mg

add_library(mg STATIC
	mg/assetsys.cpp
	mg/assetsys.h
	mg/strpool.cpp
	mg/strpool.h
)
#target_compile_definitions(mg INTERFACE
#	ASSETSYS_U64 uint64_t
#	STRPOOL_U32 uint32_t
#	STRPOOL_U64 uint64_t
#)
target_include_directories(mg INTERFACE mg)
target_link_libraries(mg PRIVATE miniz)

if(ANDROID)
	include(FetchContent)

	# SDL2

	set(SDL_TEST_ENABLED_BY_DEFAULT OFF)
	FetchContent_Declare(SDL2
		URL https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.30.1.zip
		#SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SDL2
	)

	FetchContent_MakeAvailable(SDL2)
endif()
