# OpenJazz makefile

# Sane defaults
CXX ?= g++ -std=c++11
CXXFLAGS ?= -g -Wall -O2
DEFINES = -DSCALE -DPORTABLE
CPPFLAGS = $(DEFINES) -Isrc -Iext/scale2x -Iext/psmplug -Iext/miniz -Iext/argparse

# Network support
CXXFLAGS += -DUSE_SOCKETS
ifeq ($(OS),Windows_NT)
	# Only needed under Windows.
	LIBS += -lws2_32
endif

# SDL1.2 or SDL2
SDLCONFIG ?= sdl-config
CXXFLAGS += $(shell $(SDLCONFIG) --cflags)
LIBS += $(shell $(SDLCONFIG) --libs)

LIBS += -lm

# Libraries
OJEXTLIBOBJ = \
	ext/argparse/argparse.o \
	ext/miniz/miniz.o \
	ext/psmplug/fastmix.o \
	ext/psmplug/load_psm.o \
	ext/psmplug/psmplug.o \
	ext/psmplug/snd_dsp.o \
	ext/psmplug/snd_flt.o \
	ext/psmplug/snd_fx.o \
	ext/psmplug/sndfile.o \
	ext/psmplug/sndmix.o \
	ext/scale2x/scale2x.o \
	ext/scale2x/scale3x.o \
	ext/scale2x/scalebit.o

# Main engine
OJOBJS = \
	src/game/clientgame.o \
	src/game/game.o \
	src/game/gamemode.o \
	src/game/localgame.o \
	src/game/servergame.o \
	src/io/controls.o \
	src/io/file.o \
	src/io/file_dir.o \
	src/io/log.o \
	src/io/gfx/anim.o \
	src/io/gfx/font.o \
	src/io/gfx/paletteeffects.o \
	src/io/gfx/sprite.o \
	src/io/gfx/video.o \
	src/io/network.o \
	src/io/sound.o \
	src/level/level.o \
	src/level/movable.o \
	src/main.o \
	src/menu/gamemenu.o \
	src/menu/mainmenu.o \
	src/menu/menu.o \
	src/menu/plasma.o \
	src/menu/setupmenu.o \
	src/player/player.o \
	src/setup.o \
	src/util.o \
	src/version.o

# Episode 1
OJ1OBJS = \
	src/jj1/bonuslevel/jj1bonuslevel.o \
	src/jj1/bonuslevel/jj1bonuslevelplayer.o \
	src/jj1/level/event/jj1bridge.o \
	src/jj1/level/event/jj1event.o \
	src/jj1/level/event/jj1guardians.o \
	src/jj1/level/event/jj1standardevent.o \
	src/jj1/level/jj1bird.o \
	src/jj1/level/jj1bullet.o \
	src/jj1/level/jj1demolevel.o \
	src/jj1/level/jj1level.o \
	src/jj1/level/jj1levelframe.o \
	src/jj1/level/jj1levelload.o \
	src/jj1/level/jj1levelplayer.o \
	src/jj1/level/jj1levelplayerframe.o \
	src/jj1/planet/jj1planet.o \
	src/jj1/scene/jj1scene.o \
	src/jj1/scene/jj1sceneload.o

OBJS = $(OJEXTLIBOBJ) $(OJOBJS) $(OJ1OBJS)

# Stamp
DEPR = .mk-stamp-depr

# Build rules
.PHONY: clean

OpenJazz: $(DEPR) $(OBJS)
	@-echo [LD] $@
	@$(CXX) -o OpenJazz $(LDFLAGS) $(OBJS) $(LIBS)

$(DEPR):
	@-echo [WARNING] This Makefile is deprecated! Please use CMake, if possible.
	@-echo           Report problems at https://github.com/AlisterT/openjazz/issues
	@touch $(DEPR)

%.o: %.cpp
	@-echo [CXX] $<
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	@-echo Cleaning...
	@rm -f OpenJazz $(OBJS) $(DEPR)
