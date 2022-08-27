desktopdir = $(datadir)/applications
dist_desktop_DATA = builds/unix/OpenJazz.desktop
appicondir = $(datadir)/icons/hicolor/scalable/apps
dist_appicon_DATA = builds/unix/OpenJazz.svg
appicon_fallbackdir = $(datadir)/icons/hicolor/48x48/apps
dist_appicon_fallback_DATA = builds/unix/OpenJazz.png

noinst_LIBRARIES = libpsmplug.a libminiz.a libscale2x.a libargparse.a
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
libminiz_a_SOURCES = \
	ext/miniz/miniz.cpp \
	ext/miniz/miniz.h
libscale2x_a_SOURCES = \
	ext/scale2x/scale2x.cpp \
	ext/scale2x/scale2x.h \
	ext/scale2x/scale3x.cpp \
	ext/scale2x/scale3x.h \
	ext/scale2x/scalebit.cpp \
	ext/scale2x/scalebit.h
libargparse_a_SOURCES = \
	ext/argparse/argparse.cpp \
	ext/argparse/argparse.h

bin_PROGRAMS = OpenJazz
OpenJazz_CPPFLAGS = \
	-I${srcdir}/src \
	-DSCALE \
	-I${srcdir}/ext/scale2x \
	-I${srcdir}/ext/psmplug \
	-I${srcdir}/ext/miniz \
	-I${srcdir}/ext/argparse
OpenJazz_CXXFLAGS = \
	${HOST_CFLAGS} \
	${SDL_CFLAGS}
OpenJazz_LDADD = \
	libpsmplug.a \
	libminiz.a \
	libscale2x.a \
	libargparse.a \
	${SDL_LIBS} \
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
	src/io/gfx/sprite.cpp \
	src/io/gfx/sprite.h \
	src/io/gfx/video.cpp \
	src/io/gfx/video.h \
	src/io/log.cpp \
	src/io/log.h \
	src/io/network.cpp \
	src/io/network.h \
	src/io/sound.cpp \
	src/io/sound.h \
	src/jj1/bonuslevel/jj1bonuslevel.cpp \
	src/jj1/bonuslevel/jj1bonuslevel.h \
	src/jj1/bonuslevel/jj1bonuslevelplayer.cpp \
	src/jj1/bonuslevel/jj1bonuslevelplayer.h \
	src/jj1/level/jj1bullet.cpp \
	src/jj1/level/jj1bullet.h \
	src/jj1/level/jj1demolevel.cpp \
	src/jj1/level/event/jj1bridge.cpp \
	src/jj1/level/event/jj1event.cpp \
	src/jj1/level/event/jj1event.h \
	src/jj1/level/event/jj1guardians.cpp \
	src/jj1/level/event/jj1guardians.h \
	src/jj1/level/event/jj1standardevent.cpp \
	src/jj1/level/jj1level.cpp \
	src/jj1/level/jj1levelframe.cpp \
	src/jj1/level/jj1level.h \
	src/jj1/level/jj1levelload.cpp \
	src/jj1/level//jj1bird.cpp \
	src/jj1/level//jj1bird.h \
	src/jj1/level//jj1levelplayer.cpp \
	src/jj1/level//jj1levelplayerframe.cpp \
	src/jj1/level//jj1levelplayer.h \
	src/jj1/planet/jj1planet.cpp \
	src/jj1/planet/jj1planet.h \
	src/jj1/scene/jj1scene.cpp \
	src/jj1/scene/jj1scene.h \
	src/jj1/scene/jj1sceneload.cpp \
	src/jj2/level/event/jj2event.cpp \
	src/jj2/level/event/jj2eventframe.cpp \
	src/jj2/level/event/jj2event.h \
	src/jj2/level/jj2layer.cpp \
	src/jj2/level/jj2level.cpp \
	src/jj2/level/jj2levelframe.cpp \
	src/jj2/level/jj2level.h \
	src/jj2/level/jj2levelload.cpp \
	src/jj2/level/jj2levelplayer.cpp \
	src/jj2/level/jj2levelplayerframe.cpp \
	src/jj2/level/jj2levelplayer.h \
	src/level/level.cpp \
	src/level/level.h \
	src/level/levelplayer.cpp \
	src/level/levelplayer.h \
	src/level/movable.cpp \
	src/level/movable.h \
	src/logo.h \
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

if HAVE_ASCIIDOCTOR

builds/unix/OpenJazz.6: builds/unix/OpenJazz.6.adoc
	$(AM_V_GEN)$(ASCIIDOCTOR) -b manpage -a "oj_version=$(PACKAGE_VERSION)" -o $@ $<

dist_man6_MANS = builds/unix/OpenJazz.6
MOSTLYCLEANFILES = builds/unix/OpenJazz.6

else

if HAVE_MANUAL

dist_man6_MANS = builds/unix/OpenJazz.6

endif

endif

if HAVE_SOCKETS

OpenJazz_CPPFLAGS += -DUSE_SOCKETS
OpenJazz_LDADD += ${NET_LIBS}

else

if HAVE_SDL_NET

OpenJazz_CPPFLAGS += -DUSE_SDL_NET
OpenJazz_CXXFLAGS += ${SDL_NET_CFLAGS}
OpenJazz_LDADD += ${SDL_NET_LIBS}

endif

endif

if OJ_WII

builds/wii/boot.dol: OpenJazz$(EXEEXT)
	@if ! $(AM_V_P); then echo "  ELF2DOL  $@"; fi
	$(AM_V_at)$(ELF2DOL) $< $@

all-local: builds/wii/boot.dol

clean-local:
	-rm -f builds/wii/boot.dol

endif

if OJ_RISCOS

builds/riscos/!OpenJazz/OpenJazz,ff8: OpenJazz$(EXEEXT)
	@if ! $(AM_V_P); then echo "  ELF2AIF  $@"; fi
	$(AM_V_at)$(ELF2AIF) $< $@

all-local: builds/riscos/!OpenJazz/OpenJazz,ff8

clean-local:
	$(RM) builds/riscos/!OpenJazz/OpenJazz,ff8

endif

if OJ_MINGW

.rc.o:
	@if ! $(AM_V_P); then echo "  RC       $@"; fi
	$(AM_V_at)$(WINDRES) -o $@ $<

OpenJazz_SOURCES += builds/windows/OpenJazz.rc

endif

EXTRA_DIST = \
	licenses.txt \
	README.md \
	ext/miniz/LICENSE \
	ext/psmplug/LICENSE \
	ext/scale2x/COPYING \
	doc/Doxyfile \
	doc/OpenJazz.png \
	builds/wii/icon.png \
	builds/wii/meta.xml \
	builds/wii/READMII.txt \
	builds/riscos/!OpenJazz/!Run,feb \
	builds/riscos/!OpenJazz/!Boot,feb \
	builds/riscos/!OpenJazz/!Sprites,ff9 \
	builds/windows/OpenJazz.ico \
	builds/windows/OpenJazz.rc \
	builds/unix/OpenJazz.6.adoc \
	builds/psp/Makefile \
	builds/psp/icon.png \
	builds/3ds/Makefile \
	builds/3ds/OpenJazz.png \
	src/platforms/3ds.cpp \
	src/platforms/3ds.h \
	src/platforms/psp.cpp \
	src/platforms/psp.h \
	src/platforms/psvita.cpp \
	src/platforms/psvita.h
