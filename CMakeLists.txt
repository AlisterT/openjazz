cmake_minimum_required(VERSION 3.16...3.27)

project(OpenJazz
	VERSION 20231027
	LANGUAGES CXX
	HOMEPAGE_URL http://alister.eu/jazz/oj/)

# Extra CMake Module files

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/builds/cmake)
include(CMakeDependentOption)
include(OJ-misc)
include(Platform-Helpers)

# We are in the process to migrate to C++11

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OpenJazz

add_executable(OpenJazz
	# main engine
	src/game/clientgame.cpp
	src/game/game.cpp
	src/game/game.h
	src/game/gamemode.cpp
	src/game/gamemode.h
	src/game/localgame.cpp
	src/game/servergame.cpp
	src/io/controls.cpp
	src/io/controls.h
	src/io/file.cpp
	src/io/file.h
	src/io/gfx/anim.cpp
	src/io/gfx/anim.h
	src/io/gfx/font.cpp
	src/io/gfx/font.h
	src/io/gfx/paletteeffects.cpp
	src/io/gfx/paletteeffects.h
	src/io/gfx/sprite.cpp
	src/io/gfx/sprite.h
	src/io/gfx/video.cpp
	src/io/gfx/video.h
	src/io/log.cpp
	src/io/log.h
	src/io/network.cpp
	src/io/network.h
	src/io/sound.cpp
	src/io/sound.h
	src/level/level.cpp
	src/level/level.h
	src/level/levelplayer.h
	src/level/movable.cpp
	src/level/movable.h
	src/logo.h
	src/loop.h
	src/main.cpp
	src/menu/gamemenu.cpp
	src/menu/mainmenu.cpp
	src/menu/menu.cpp
	src/menu/menu.h
	src/menu/plasma.cpp
	src/menu/plasma.h
	src/menu/setupmenu.cpp
	src/OpenJazz.h
	src/platforms/platforms.h
	src/player/player.cpp
	src/player/player.h
	src/setup.cpp
	src/setup.h
	src/util.cpp
	src/util.h
	src/version.cpp
	src/version.h
	# episode 1
	src/jj1/bonuslevel/jj1bonuslevel.cpp
	src/jj1/bonuslevel/jj1bonuslevel.h
	src/jj1/bonuslevel/jj1bonuslevelplayer.cpp
	src/jj1/bonuslevel/jj1bonuslevelplayer.h
	src/jj1/level//jj1bird.cpp
	src/jj1/level//jj1bird.h
	src/jj1/level//jj1levelplayer.cpp
	src/jj1/level//jj1levelplayer.h
	src/jj1/level//jj1levelplayerframe.cpp
	src/jj1/level/event/jj1bridge.cpp
	src/jj1/level/event/jj1event.cpp
	src/jj1/level/event/jj1event.h
	src/jj1/level/event/jj1guardians.cpp
	src/jj1/level/event/jj1guardians.h
	src/jj1/level/event/jj1standardevent.cpp
	src/jj1/level/jj1bullet.cpp
	src/jj1/level/jj1bullet.h
	src/jj1/level/jj1demolevel.cpp
	src/jj1/level/jj1level.cpp
	src/jj1/level/jj1level.h
	src/jj1/level/jj1levelframe.cpp
	src/jj1/level/jj1levelload.cpp
	src/jj1/planet/jj1planet.cpp
	src/jj1/planet/jj1planet.h
	src/jj1/scene/jj1scene.cpp
	src/jj1/scene/jj1scene.h
	src/jj1/scene/jj1sceneload.cpp)
target_include_directories(OpenJazz PUBLIC src)

# portable mode

cmake_dependent_option(PORTABLE "Do not write to external directories, etc." OFF
	"NOT OJ_DEFAULT_PORTABLE" ON)
if(PORTABLE)
	set(PORTABLE_STATUS "Enabled")
	target_compile_definitions(OpenJazz PRIVATE PORTABLE)
else()
	set(PORTABLE_STATUS "Disabled")
endif()
if((PORTABLE AND OJ_DEFAULT_PORTABLE) OR (NOT PORTABLE AND NOT OJ_DEFAULT_PORTABLE))
	set(PORTABLE_STATUS "${PORTABLE_STATUS} (Default)")
endif()

# path to game data

set(DATAPATH "" CACHE PATH "Fixed path where game data is available")
if(DATAPATH)
	target_compile_definitions(OpenJazz PRIVATE DATAPATH="${DATAPATH}")
endif()

# libraries

if(LEGACY_SDL)
	find_package(SDL REQUIRED)
	target_include_directories(OpenJazz PRIVATE ${SDL_INCLUDE_DIR})
	set(OJ_LIBS_SDL ${SDL_LIBRARY})
	set(SDL_STATUS "SDL1.2 (legacy)")
else()
	find_package(SDL2 REQUIRED)
	target_include_directories(OpenJazz PRIVATE ${SDL2_INCLUDE_DIRS})
	set(OJ_LIBS_SDL ${SDL2_LIBRARIES})
	set(SDL_STATUS "SDL2")
endif()

# common bundled libraries

add_subdirectory(ext)

# network

set(NETWORK_STATUS "Disabled")
option(NETWORK "Enable Network support" ON)
if(NETWORK)
	set(NETWORK_STATUS "Enabled, platform specific")
	if(NOT OJ_LIBS_NET)
		# sockets
		include(CheckIncludeFileCXX)
		check_include_file_cxx(sys/socket.h HAVE_SOCKETS)
		if(HAVE_SOCKETS)
			set(NETWORK_STATUS "Enabled, sockets")
			target_compile_definitions(OpenJazz PRIVATE USE_SOCKETS)
		else()
			# SDL_net
			find_package(SDL_net)
			if(SDL_NET_FOUND)
				set(NETWORK_STATUS "Enabled, SDL_net")
				target_compile_definitions(OpenJazz PRIVATE USE_SDL_NET)
			else()
				set(NETWORK_STATUS "Disabled - no sockets or SDL_net found")
			endif()
		endif()
	endif()
endif()

# scaling

set(SCALE_STATUS "Disabled")
cmake_dependent_option(SCALE "Allow scaling" ON "OJ_ALLOW_SCALE;LEGACY_SDL" OFF)
if(SCALE)
	set(SCALE_STATUS "Enabled, scale2x")
	target_compile_definitions(OpenJazz PRIVATE SCALE)
	set(OJ_LIBS_SCALE scale2x)
	add_subdirectory(ext/scale2x)
elseif(NOT LEGACY_SDL)
	# FIXME: add codepath for gpu scaling
	set(SCALE_STATUS "Disabled (currently broken in SDL2 port)")
endif()

option(ENABLE_JJ2 "Enable experimental Episode 2 support (not recommended)" OFF)
if(ENABLE_JJ2)
	target_sources(OpenJazz PRIVATE
		src/jj2/level/event/jj2event.cpp
		src/jj2/level/event/jj2event.h
		src/jj2/level/event/jj2eventframe.cpp
		src/jj2/level/jj2layer.cpp
		src/jj2/level/jj2level.cpp
		src/jj2/level/jj2level.h
		src/jj2/level/jj2levelframe.cpp
		src/jj2/level/jj2levelload.cpp
		src/jj2/level/jj2levelplayer.cpp
		src/jj2/level/jj2levelplayer.h
		src/jj2/level/jj2levelplayerframe.cpp)
	target_compile_definitions(OpenJazz PRIVATE ENABLE_JJ2)
endif()

# version

string(TIMESTAMP OJ_DATE "%Y-%m-%d")
include(GetGitRevisionDescription)
git_get_exact_tag(GIT_TAG)
# Do not include a hash, if we are building a release tag
if(NOT GIT_TAG)
	# otherwise concatenate a version with hash
	git_describe(GIT_DESCRIPTION)
	if(GIT_DESCRIPTION)
		string(REPLACE "-" ";" GIT_DESCRIPTION ${GIT_DESCRIPTION})
		list(LENGTH GIT_DESCRIPTION GIT_DESCRIPTION_PARTS)
		if(GIT_DESCRIPTION_PARTS EQUAL 3)
			list(GET GIT_DESCRIPTION 0 GIT_TAG)
			list(GET GIT_DESCRIPTION 1 GIT_COMMITS)
			list(GET GIT_DESCRIPTION 2 GIT_HASH)
			set(GIT_STATUS "${GIT_COMMITS} commits since tag \"${GIT_TAG}\", ")
			string(PREPEND GIT_COMMITS "+")
		else()
			# no tags found, only hash
			list(GET GIT_DESCRIPTION 0 GIT_HASH)
		endif()
		# strip the g prefix
		string(SUBSTRING ${GIT_HASH} 1 -1 GIT_HASH)
		set(OJ_VERSION_GIT "git${GIT_COMMITS}@${GIT_HASH}")
		string(APPEND GIT_STATUS "object hash is ${GIT_HASH}")
		git_local_changes(GIT_DIRTY)
		if(GIT_DIRTY STREQUAL "DIRTY")
			string(APPEND OJ_VERSION_GIT "-dirty")
			string(APPEND GIT_STATUS ", with uncommitted changes")
		endif()
	endif()
endif()
set_property(SOURCE src/version.cpp PROPERTY COMPILE_DEFINITIONS
	OJ_VERSION="${PROJECT_VERSION}"; OJ_DATE="${OJ_DATE}";
	$<$<BOOL:${OJ_VERSION_GIT}>:OJ_VERSION_GIT="${OJ_VERSION_GIT}">)

# project links

set(OJ_BUGREPORT "https://github.com/AlisterT/openjazz/issues")
set_property(SOURCE src/main.cpp PROPERTY COMPILE_DEFINITIONS
	OJ_URL="${PROJECT_HOMEPAGE_URL}"; OJ_BUGREPORT="${OJ_BUGREPORT}")

target_link_libraries(OpenJazz
	argparse miniz psmplug ${OJ_LIBS_SCALE} ${OJ_LIBS_SDL} ${OJ_LIBS_NET} ${OJ_LIBS_HOST})

# platform stuff

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(OpenJazz PRIVATE "-fno-math-errno")
endif()

if(WIN32)
	# open console for debug builds
	set_target_properties(OpenJazz PROPERTIES WIN32_EXECUTABLE $<$<NOT:$<CONFIG:Debug>>:TRUE>)
	# add icon
	target_sources(OpenJazz PRIVATE res/windows/OpenJazz.rc)
elseif(RISCOS)
	target_sources(OpenJazz PRIVATE
		src/platforms/riscos.cpp
		src/platforms/riscos.h)
	target_link_options(OpenJazz PRIVATE "-static")
	elf2aif(OpenJazz)
elseif(3DS)
	target_sources(OpenJazz PRIVATE
		src/platforms/3ds.cpp
		src/platforms/3ds.h)
	target_link_libraries(OpenJazz -lcitro3d)
	ctr_generate_smdh(OpenJazz.smdh
		NAME "OpenJazz"
		DESCRIPTION "Jack Jazzrabbit 1 game engine reimplementation"
		AUTHOR "AlisterT & carstene1ns"
		ICON ${CMAKE_CURRENT_SOURCE_DIR}/res/3ds/OpenJazz.png)
	ctr_create_3dsx(OpenJazz
		SMDH OpenJazz.smdh
		${ROMFS_ARG} ${ROMFS_PATH})
elseif(WII)
	target_sources(OpenJazz PRIVATE
		src/platforms/wii.cpp
		src/platforms/wii.h)
	target_link_libraries(OpenJazz -laesnd -lfat -lwiikeyboard)
	ogc_create_dol(OpenJazz)
elseif(PSP)
	target_sources(OpenJazz PRIVATE
		src/platforms/psp.cpp
		src/platforms/psp.h)
	target_link_libraries(OpenJazz
		-lGL -lpspdebug -lpspgu -lpspctrl -lpspge -lpspdisplay -lpsphprm -lpspvfpu -lpspaudio -lpspirkeyb -lpsppower)
	create_pbp_file(TARGET OpenJazz
		TITLE "\"Jazz Jackrabbit (OpenJazz)\""
		ICON_PATH ${CMAKE_CURRENT_SOURCE_DIR}/res/psp/icon.png
		BUILD_PRX)
elseif(PSVITA) # unfinished
	target_sources(OpenJazz PRIVATE
		src/platforms/psvita.cpp
		src/platforms/psvita.h)
elseif(GP2X OR WIZ)
	target_sources(OpenJazz PRIVATE
		src/platforms/wiz.cpp
		src/platforms/wiz.h)
elseif(HAIKU)
	target_sources(OpenJazz PRIVATE
		src/platforms/haiku.cpp
		src/platforms/haiku.h)
elseif(UNIX)
	target_sources(OpenJazz PRIVATE
		src/platforms/xdg.cpp
		src/platforms/xdg.h)
	target_link_libraries(OpenJazz -lm)
endif()

# installation

if(WIN32)
	# put everything in a folder
	install(TARGETS OpenJazz DESTINATION dist)
elseif(3DS)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/OpenJazz.3dsx
		DESTINATION OpenJazz COMPONENT 3ds)
elseif(WII)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/OpenJazz.dol
		RENAME boot.dol
		DESTINATION OpenJazz COMPONENT wii)
	install(FILES res/wii/icon.png res/wii/meta.xml res/wii/READMII.txt
		DESTINATION OpenJazz COMPONENT wii)
elseif(PSP)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/EBOOT.PBP
		DESTINATION OpenJazz COMPONENT psp)
	install(TARGETS OpenJazz RUNTIME DESTINATION . COMPONENT debug)
elseif(RISCOS)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/OpenJazz,ff8
		res/riscos/!Run,feb res/riscos/!Boot,feb res/riscos/!Sprites,ff9
		DESTINATION !OpenJazz COMPONENT riscos)
elseif(PANDORA OR CANOO OR WIZ OR GP2X OR DINGOO OR GAMESHELL)
	# left blank for the moment (TODO: opk?)
elseif(UNIX)
	include(GNUInstallDirs)

	install(TARGETS OpenJazz RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
	install(FILES res/unix/OpenJazz.png DESTINATION ${CMAKE_INSTALL_DATADIR}/pixmaps)
	install(FILES res/unix/OpenJazz.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/apps)
	install(FILES res/unix/OpenJazz.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
	install(FILES res/unix/OpenJazz.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
else()
	message(VERBOSE "No idea how to install for your platform")
endif()

# debug information

if(3DS OR WII)
	install(TARGETS OpenJazz RUNTIME DESTINATION . COMPONENT debug)
	dkp_target_generate_symbol_list(OpenJazz)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/OpenJazz.map
			${CMAKE_CURRENT_BINARY_DIR}/OpenJazz.lst
		DESTINATION . COMPONENT debug)
endif()

# packaging for distribution

set(CPACK_GENERATOR "ZIP")
set(CPACK_SOURCE_GENERATOR ";") # disable
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON) # split archives
set(CPACK_PACKAGE_FILE_NAME "OpenJazz-${PROJECT_VERSION}")
set(CPACK_ARCHIVE_DEBUG_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-debug")
set(CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY OFF) # we do this manually
include(CPack)

# manual page

find_program(ASCIIDOCTOR_EXECUTABLE asciidoctor)
set(MANUAL_STATUS "Unavailable")
set(MAN_PATH "res/unix/OpenJazz.6")
if(ASCIIDOCTOR_EXECUTABLE)
	add_custom_command(OUTPUT ${MAN_PATH}
		COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/res/unix
		COMMAND ${ASCIIDOCTOR_EXECUTABLE} -a oj_version="${PROJECT_VERSION}"
			-b manpage -o ${CMAKE_CURRENT_BINARY_DIR}/${MAN_PATH}
			${CMAKE_CURRENT_SOURCE_DIR}/${MAN_PATH}.adoc
		DEPENDS ${MAN_PATH}.adoc
		COMMENT "(Re-)building manual page"
		VERBATIM)
	if(UNIX)
		add_custom_target(man ALL DEPENDS ${MAN_PATH})
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${MAN_PATH}
			DESTINATION ${CMAKE_INSTALL_MANDIR}/man6)
		set(MANUAL_STATUS "Generated")
	else()
		add_custom_target(man DEPENDS ${MAN_PATH})
		set(MANUAL_STATUS "Optional")
	endif()
else()
	# distribution archive?
	if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${MAN_PATH})
		if(UNIX)
			install(FILES ${MAN_PATH} DESTINATION ${CMAKE_INSTALL_MANDIR}/man6)
		endif()
		set(MANUAL_STATUS "Bundled")
	endif()
endif()
unset(MAN_PATH)

# Print short summary
message(STATUS "")
message(STATUS "OpenJazz")
message(STATUS "========")
message(STATUS "Version: ${PROJECT_VERSION}")
if(GIT_STATUS)
	message(STATUS "Git status: ${GIT_STATUS}")
endif()
message(STATUS "Target system: ${OJ_HOST}")
message(STATUS "Platform abstraction: ${SDL_STATUS}")
if(ROMFS)
	message(STATUS "RomFS: Embedding directory \"${ROMFS_PATH}\"")
endif()
message(STATUS "Network: ${NETWORK_STATUS}")
message(STATUS "Scaling: ${SCALE_STATUS}")
if(DATAPATH)
	message(STATUS "Additional/System Game Data Path: \"${DATAPATH}\"")
endif()
message(STATUS "Manual page: ${MANUAL_STATUS}")
message(STATUS "Portable Engine: ${PORTABLE_STATUS}")
message(STATUS "")
message(STATUS "In case something goes wrong, report bugs to ${OJ_BUGREPORT}")
message(STATUS "")
