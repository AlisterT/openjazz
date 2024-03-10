
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

# lodepng

add_library(lodepng STATIC lodepng/lodepng.h lodepng/lodepng.cpp)
target_compile_definitions(lodepng INTERFACE LODEPNG_NO_COMPILE_DISK)
target_include_directories(lodepng INTERFACE lodepng)
