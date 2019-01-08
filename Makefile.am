dist_pkgdata_DATA = openjazz.000

desktopdir = $(datadir)/applications
dist_desktop_DATA = unix/OpenJazz.desktop
appicondir = $(datadir)/icons/hicolor/scalable/apps
dist_appicon_DATA = unix/OpenJazz.svg
appicon_fallbackdir = $(datadir)/icons/hicolor/48x48/apps
dist_appicon_fallback_DATA = unix/OpenJazz.png

noinst_LIBRARIES = libpsmplug.a
libpsmplug_a_SOURCES = \
	ext/psmplug/fastmix.cpp \
	ext/psmplug/load_psm.cpp \
	ext/psmplug/psmplug.cpp \
	ext/psmplug/psmplug.h \
	ext/psmplug/snd_dsp.cpp \
	ext/psmplug/sndfile.cpp \
	ext/psmplug/sndfile.h \
	ext/psmplug/snd_flt.cpp \
	ext/psmplug/snd_fx.cpp \
	ext/psmplug/sndmix.cpp \
	ext/psmplug/stdafx.h \
	ext/psmplug/tables.h

bin_PROGRAMS = OpenJazz
OpenJazz_CPPFLAGS = \
	-DSCALE \
	-I${srcdir}/src \
	-I${srcdir}/ext/psmplug -DUSE_PSMPLUG
OpenJazz_CXXFLAGS = \
	${HOST_CFLAGS} \
	${SDL_CFLAGS} \
	${ZLIB_CFLAGS}
OpenJazz_LDADD = \
	${SDL_LIBS} \
	libpsmplug.a \
	${ZLIB_LIBS} \
	${NET_LIBS} \
	${HOST_LIBS}
OpenJazz_SOURCES = \
	src/game/clientgame.cpp \
	src/game/game.cpp \
	src/game/game.h \
	src/game/gamemode.cpp \
	src/game/gamemode.h \
	src/game/localgame.cpp \
	src/game/servergame.cpp \
	src/io/controls.cpp \
	src/io/controls.h \
	src/io/file.cpp \
	src/io/file.h \
	src/io/gfx/anim.cpp \
	src/io/gfx/anim.h \
	src/io/gfx/font.cpp \
	src/io/gfx/font.h \
	src/io/gfx/paletteeffects.cpp \
	src/io/gfx/paletteeffects.h \
	src/io/gfx/scale2x/getopt.cpp \
	src/io/gfx/scale2x/pixel.cpp \
	src/io/gfx/scale2x/pixel.h \
	src/io/gfx/scale2x/portable.h \
	src/io/gfx/scale2x/scale2x.cpp \
	src/io/gfx/scale2x/scale2x.h \
	src/io/gfx/scale2x/scale3x.cpp \
	src/io/gfx/scale2x/scale3x.h \
	src/io/gfx/scale2x/scalebit.cpp \
	src/io/gfx/scale2x/scalebit.h \
	src/io/gfx/scale2x/simple2x.cpp \
	src/io/gfx/sprite.cpp \
	src/io/gfx/sprite.h \
	src/io/gfx/video.cpp \
	src/io/gfx/video.h \
	src/io/network.cpp \
	src/io/network.h \
	src/io/sound.cpp \
	src/io/sound.h \
	src/jj1bonuslevel/jj1bonuslevel.cpp \
	src/jj1bonuslevel/jj1bonuslevel.h \
	src/jj1bonuslevel/jj1bonuslevelplayer/jj1bonuslevelplayer.cpp \
	src/jj1bonuslevel/jj1bonuslevelplayer/jj1bonuslevelplayer.h \
	src/jj1level/jj1bullet.cpp \
	src/jj1level/jj1bullet.h \
	src/jj1level/jj1demolevel.cpp \
	src/jj1level/jj1event/jj1bridge.cpp \
	src/jj1level/jj1event/jj1event.cpp \
	src/jj1level/jj1event/jj1event.h \
	src/jj1level/jj1event/jj1guardians.cpp \
	src/jj1level/jj1event/jj1guardians.h \
	src/jj1level/jj1event/jj1standardevent.cpp \
	src/jj1level/jj1level.cpp \
	src/jj1level/jj1levelframe.cpp \
	src/jj1level/jj1level.h \
	src/jj1level/jj1levelload.cpp \
	src/jj1level/jj1levelplayer/jj1bird.cpp \
	src/jj1level/jj1levelplayer/jj1bird.h \
	src/jj1level/jj1levelplayer/jj1levelplayer.cpp \
	src/jj1level/jj1levelplayer/jj1levelplayerframe.cpp \
	src/jj1level/jj1levelplayer/jj1levelplayer.h \
	src/jj1planet/jj1planet.cpp \
	src/jj1planet/jj1planet.h \
	src/jj1scene/jj1scene.cpp \
	src/jj1scene/jj1scene.h \
	src/jj1scene/jj1sceneload.cpp \
	src/jj2level/jj2event/jj2event.cpp \
	src/jj2level/jj2event/jj2eventframe.cpp \
	src/jj2level/jj2event/jj2event.h \
	src/jj2level/jj2layer.cpp \
	src/jj2level/jj2level.cpp \
	src/jj2level/jj2levelframe.cpp \
	src/jj2level/jj2level.h \
	src/jj2level/jj2levelload.cpp \
	src/jj2level/jj2levelplayer/jj2levelplayer.cpp \
	src/jj2level/jj2levelplayer/jj2levelplayerframe.cpp \
	src/jj2level/jj2levelplayer/jj2levelplayer.h \
	src/level/level.cpp \
	src/level/level.h \
	src/level/levelplayer.cpp \
	src/level/levelplayer.h \
	src/level/movable.cpp \
	src/level/movable.h \
	src/loop.h \
	src/main.cpp \
	src/menu/gamemenu.cpp \
	src/menu/mainmenu.cpp \
	src/menu/menu.cpp \
	src/menu/menu.h \
	src/menu/plasma.cpp \
	src/menu/plasma.h \
	src/menu/setupmenu.cpp \
	src/OpenJazz.h \
	src/platforms/wii.cpp \
	src/platforms/wii.h \
	src/platforms/wiz.cpp \
	src/platforms/wiz.h \
	src/player/player.cpp \
	src/player/player.h \
	src/setup.cpp \
	src/setup.h \
	src/util.cpp \
	src/util.h

if HAVE_POD2MAN

unix/OpenJazz.6: unix/OpenJazz.6.pod
	$(AM_V_GEN)$(POD2MAN) -c "OpenJazz Manual" -n OpenJazz -r "OpenJazz $(PACKAGE_VERSION)" $< $@

dist_man6_MANS = unix/OpenJazz.6
MOSTLYCLEANFILES = unix/OpenJazz.6

else

if HAVE_MANUAL

dist_man6_MANS = unix/OpenJazz.6

endif

endif

if OJ_WII

OpenJazz_CPPFLAGS += -DUSE_SDL_NET

wii/boot.dol: OpenJazz$(EXEEXT)
	$(AM_V_GEN) elf2dol $< $@

all-local: wii/boot.dol

clean-local:
	-rm -f wii/boot.dol

else

OpenJazz_CPPFLAGS += -DUSE_SOCKETS

endif

if OJ_RISCOS

riscos/!OpenJazz/OpenJazz,ff8: OpenJazz$(EXEEXT)
	$(AM_V_GEN) elf2aif $< $@

all-local: riscos/!OpenJazz/OpenJazz,ff8

clean-local:
	$(RM) riscos/!OpenJazz/OpenJazz,ff8

endif

if OJ_MINGW

.rc.o:
	@if ! $(AM_V_P); then echo "  RC       $@"; fi
	$(AM_V_at)$(WINDRES) -o $@ $<

OpenJazz_SOURCES += windows/OpenJazz.rc

endif

EXTRA_DIST = \
	gpl.txt \
	licenses.txt \
	README.md \
	doc/Doxyfile \
	doc/OpenJazz.png \
	wii/icon.png \
	wii/meta.xml \
	wii/READMII.txt \
	riscos/!OpenJazz/!Run,feb \
	riscos/!OpenJazz/!Boot,feb \
	riscos/!OpenJazz/!Sprites,ff9 \
	windows/OpenJazz.ico \
	windows/OpenJazz.rc \
	unix/OpenJazz.6.pod
