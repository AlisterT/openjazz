
enable_language(C)
set(CMAKE_C_STANDARD 99)

include(CheckSymbolExists)
include(CheckFunctionExists)

# xmp

add_library(xmp STATIC
	xmp.h
	src/callbackio.h
	src/common.h
	src/control.c
	src/dataio.c
	src/effects.c
	src/effects.h
	src/filetype.c
	src/filter.c
	src/flow.c
	src/format.c
	src/format.h
	src/hio.c
	src/hio.h
	src/lfo.c
	src/lfo.h
	src/load.c
	src/load_helpers.c
	src/mdataio.h
	src/memio.c
	src/memio.h
	src/misc.c
	src/mix_all.c
	src/mix_all.h
	src/mixer.c
	src/mixer.h
	src/period.c
	src/period.h
	src/player.c
	src/player.h
	src/precomp_lut.h
	src/read_event.c
	src/rng.c
	src/rng.h
	src/scan.c
	src/smix.c
	src/virtual.c
	src/virtual.h
	src/win32.c
	src/loaders/common.c
	src/loaders/iff.c
	src/loaders/iff.h
	src/loaders/loader.h
	src/loaders/masi_load.c
	src/loaders/sample.c)
target_include_directories(xmp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(xmp PRIVATE LIBXMP_CORE_PLAYER LIBXMP_CORE_DISABLE_IT)

if(UNIX)
	set(CMAKE_REQUIRED_LIBRARIES "m")
	check_symbol_exists(powf "math.h" HAVE_POWF)
	if(HAVE_POWF)
		target_compile_definitions(xmp PRIVATE HAVE_POWF)
	endif()
	target_link_libraries(xmp PRIVATE m)
endif()

target_link_libraries(OpenJazz xmp)
