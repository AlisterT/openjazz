# -*- Makefile -*-

AM_CPPFLAGS = -DUSE_MODPLUG -DUSE_SOCKETS -DSCALE -I${top_srcdir}/src \
              ${libSDL_CFLAGS} ${libmodplug_CFLAGS}
AM_CFLAGS   = -Wall
AM_CXXFLAGS = -Wall

pkgdata_DATA = openjazz.000

bin_PROGRAMS = openjazz
openjazz_SOURCES = \
	src/game/clientgame.cpp src/game/game.cpp src/game/gamemode.cpp \
	src/game/localgame.cpp src/game/servergame.cpp \
	src/io/gfx/anim.cpp src/io/gfx/font.cpp src/io/gfx/paletteeffects.cpp \
	src/io/gfx/sprite.cpp src/io/gfx/video.cpp \
	src/io/gfx/scale2x/getopt.cpp src/io/gfx/scale2x/pixel.cpp \
	src/io/gfx/scale2x/scale2x.cpp src/io/gfx/scale2x/scale3x.cpp \
	src/io/gfx/scale2x/scalebit.cpp src/io/gfx/scale2x/simple2x.cpp \
	src/io/controls.cpp src/io/file.cpp src/io/network.cpp src/io/sound.cpp \
	src/jj1bonuslevel/jj1bonuslevelplayer/jj1bonuslevelplayer.cpp \
	src/jj1bonuslevel/jj1bonuslevel.cpp \
	src/jj1level/jj1event/jj1bridge.cpp src/jj1level/jj1event/jj1event.cpp \
	src/jj1level/jj1event/jj1guardians.cpp \
	src/jj1level/jj1event/jj1standardevent.cpp \
	src/jj1level/jj1levelplayer/jj1bird.cpp \
	src/jj1level/jj1levelplayer/jj1levelplayer.cpp \
	src/jj1level/jj1levelplayer/jj1levelplayerframe.cpp \
	src/jj1level/jj1bullet.cpp src/jj1level/jj1demolevel.cpp \
	src/jj1level/jj1level.cpp src/jj1level/jj1levelframe.cpp \
	src/jj1level/jj1levelload.cpp \
	src/jj1planet/jj1planet.cpp \
	src/jj1scene/jj1scene.cpp src/jj1scene/jj1sceneload.cpp \
	src/jj2level/jj2event/jj2event.cpp src/jj2level/jj2event/jj2eventframe.cpp \
	src/jj2level/jj2levelplayer/jj2levelplayer.cpp \
	src/jj2level/jj2levelplayer/jj2levelplayerframe.cpp \
	src/jj2level/jj2layer.cpp src/jj2level/jj2level.cpp \
	src/jj2level/jj2levelframe.cpp src/jj2level/jj2levelload.cpp \
	src/level/level.cpp src/level/movable.cpp src/level/levelplayer.cpp \
	src/menu/gamemenu.cpp src/menu/mainmenu.cpp src/menu/menu.cpp \
	src/menu/plasma.cpp src/menu/setupmenu.cpp \
	src/player/player.cpp \
	src/main.cpp src/setup.cpp src/util.cpp
openjazz_LDADD = -lz ${libSDL_LIBS} ${libmodplug_LIBS}
