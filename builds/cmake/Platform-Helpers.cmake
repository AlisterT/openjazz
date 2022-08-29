
set(OJ_HOST "Unknown")
set(PLATFORM_LIST "")
set(OJ_SCALE ON)
set(OJ_LIBS_NET)
set(OJ_LIBS_HOST)

option(PANDORA "Build for Pandora" OFF)     # arm-none-linux-gnueabi
option(CAANOO "Build for GP2X Canoo" OFF)   # arm-gph-linux-gnueabi
option(WIZ "Build for GP2X Wiz" OFF)        # arm-openwiz-linux-gnu
option(GP2X "Build for GP2X" OFF)           # arm-open2x-linux
option(DINGOO "Build for Dingoo" OFF)       # mipsel-linux*
option(GAMESHELL "Build for GameShell" OFF) # armv7l-unknown-linux-gnueabihf
option(RISCOS "Build for RISC OS" OFF)      # arm-unknown-riscos
# "Official" Toolchain files define these
if(NINTENDO_3DS)
	set(3DS ON)
	set(OJ_HOST "3DS")
	list(APPEND PLATFORM_LIST ${OJ_HOST})
	set(OJ_SCALE OFF)
	option(ROMFS "Embedd a directory in the executable" OFF)
	set(ROMFS_PATH "romfs" CACHE PATH "Directory to include in executable as romfs:/ path")
	set(ROMFS_ARG "NO_ROMFS_IGNORE_ME")
	if(ROMFS)
		set(ROMFS_ARG "ROMFS")
	endif()
elseif(NINTENDO_WII)
	set(WII ON)
	set(OJ_HOST "Wii")
	list(APPEND PLATFORM_LIST ${OJ_HOST})
elseif(HAIKU)
	add_compile_definitions(_BSD_SOURCE)
	set(OJ_LIBS_HOST "-lbe")
	set(OJ_LIBS_NET "-lnetwork")
	set(OJ_HOST "Haiku")
	list(APPEND PLATFORM_LIST ${OJ_HOST})
elseif(WIN32)
	set(OJ_LIBS_NET "-lws2_32")
elseif(PSP)
	set(OJ_HOST "PSP")
	set(OJ_SCALE OFF)
endif()

if(PANDORA OR CANOO OR WIZ OR GP2X)
	add_compile_options(-fsigned-char)
endif()

if(PANDORA)
	add_compile_definitions(PANDORA)
	set(OJ_LIBS_HOST "-lts")
	set(OJ_HOST "Pandora")
	list(APPEND PLATFORM_LIST ${OJ_HOST})
elseif(CAANOO)
	add_compile_definitions(CAANOO)
	set(OJ_HOST "GP2X Caanoo")
	list(APPEND PLATFORM_LIST ${OJ_HOST})
elseif(WIZ)
	add_compile_definitions(WIZ)
	set(OJ_HOST "GP2X Wiz")
	list(APPEND PLATFORM_LIST ${OJ_HOST})
elseif(GP2X)
	add_compile_definitions(GP2X)
	set(OJ_HOST "GP2X")
	list(APPEND PLATFORM_LIST ${OJ_HOST})
elseif(DINGOO)
	add_compile_definitions(DINGOO)
	set(OJ_HOST "Dingoo")
	list(APPEND PLATFORM_LIST ${OJ_HOST})
elseif(GAMESHELL)
	add_compile_definitions(GAMESHELL)
	set(OJ_HOST "ClockworkPi GameShell")
	list(APPEND PLATFORM_LIST ${OJ_HOST})
elseif(RISCOS)
	set(OJ_HOST "RISC OS")
	list(APPEND PLATFORM_LIST ${OJ_HOST})

	find_program(ELF2AIF_EXECUTABLE elf2aif)
	function(elf2aif target)
		get_target_property(TARGET_BINARY_DIR  ${target} BINARY_DIR)
		get_target_property(TARGET_OUTPUT_NAME ${target} OUTPUT_NAME)
		if(NOT TARGET_OUTPUT_NAME)
			set(TARGET_OUTPUT_NAME "${target}")
		endif()
		set(AIF_OUTPUT "${TARGET_BINARY_DIR}/${TARGET_OUTPUT_NAME},ff8")

		add_custom_command(TARGET ${target} POST_BUILD
			COMMAND "${ELF2AIF_EXECUTABLE}" "$<TARGET_FILE:${target}>" "${AIF_OUTPUT}"
			BYPRODUCTS "${AIF_OUTPUT}"
			COMMENT "Converting ${target} to AIF format"
			VERBATIM)
	endfunction()
endif()

# sanity check

list(LENGTH PLATFORM_LIST NUM_PLATFORMS)
if(${NUM_PLATFORMS} GREATER 1)
	if(${CMAKE_VERSION} VERSION_LESS 3.12)
		set(PLATFORMS ${PLATFORM_LIST})
	else()
		list(JOIN PLATFORM_LIST ", " PLATFORMS)
	endif()
	message(FATAL_ERROR "Can only target one platform at a time! Got ${PLATFORMS}.")
endif()

# autodetect

if(${OJ_HOST} STREQUAL "Unknown")
	set(OJ_HOST ${CMAKE_SYSTEM_NAME})
endif()
