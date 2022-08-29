
# argparse

add_library(argparse STATIC argparse/argparse.h argparse/argparse.cpp)
target_include_directories(argparse PUBLIC argparse)

# miniz

add_library(miniz STATIC miniz/miniz.h miniz/miniz.cpp)
target_compile_definitions(miniz PUBLIC	MINIZ_NO_ARCHIVE_APIS)
#target_compile_definitions(miniz PRIVATE _LARGEFILE64_SOURCE)
target_include_directories(miniz PUBLIC miniz)

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

if(SCALE)
	# scale2x

	include(Find-Restrict-Keyword)

	add_library(scale2x STATIC
		scale2x/scale2x.cpp
		scale2x/scale2x.h
		scale2x/scale3x.cpp
		scale2x/scale3x.h
		scale2x/scalebit.cpp
		scale2x/scalebit.h
	)
	target_compile_definitions(scale2x PRIVATE
		USE_SCALE_RANDOMWRITE
		restrict=${RESTRICT_KEYWORD})
	target_include_directories(scale2x PUBLIC scale2x)
endif()
