
enable_language(C)
set(CMAKE_C_STANDARD 99)

# argparse

add_library(argparse STATIC argparse/argparse.h argparse/argparse.c)
target_include_directories(argparse INTERFACE argparse)
target_link_libraries(OpenJazz argparse)

# miniz

add_library(miniz STATIC miniz/miniz.h miniz/miniz.c)
target_compile_definitions(miniz INTERFACE MINIZ_NO_ARCHIVE_APIS)
#target_compile_definitions(miniz PRIVATE _LARGEFILE64_SOURCE)
target_include_directories(miniz INTERFACE miniz)
target_link_libraries(OpenJazz miniz)

# xmp

add_subdirectory(xmp)

# stb

add_library(stb STATIC stb/stb_rect_pack.h stb/stb_rect_pack.c)
target_include_directories(stb INTERFACE stb)
target_link_libraries(OpenJazz stb)

if(ANDROID)
	include(FetchContent)

	# SDL2
	set(SDL_TEST_ENABLED_BY_DEFAULT OFF)
	FetchContent_Declare(SDL2
		URL https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.32.10.zip
		#SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SDL2
	)
	FetchContent_MakeAvailable(SDL2)
	target_link_libraries(OpenJazz SDL2::SDL2)
endif()
