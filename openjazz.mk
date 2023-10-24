
# libraries
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

# main engine
OJOBJS = \
	src/game/clientgame.o \
	src/game/game.o \
	src/game/gamemode.o \
	src/game/localgame.o \
	src/game/servergame.o \
	src/io/controls.o \
	src/io/file.o \
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
	src/util.o

# episode 1
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

# episode 2
OJ2OBJS = \
	src/jj2/level/event/jj2event.o \
	src/jj2/level/event/jj2eventframe.o \
	src/jj2/level/jj2layer.o \
	src/jj2/level/jj2level.o \
	src/jj2/level/jj2levelframe.o \
	src/jj2/level/jj2levelload.o \
	src/jj2/level/jj2levelplayer.o \
	src/jj2/level/jj2levelplayerframe.o

OBJS = $(OJEXTLIBOBJ) $(OJOBJS) $(OJ1OBJS) $(OJ2OBJS)
