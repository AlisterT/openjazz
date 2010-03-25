# OpenJazz makefile

objects = src/bonus/bonus.o \
	src/game/clientgame.o src/game/game.o src/game/gamemode.o \
	src/game/servergame.o \
	src/io/gfx/anim.o src/io/gfx/font.o src/io/gfx/paletteeffects.o \
	src/io/gfx/sprite.o src/io/gfx/video.o \
	src/io/controls.o src/io/file.o src/io/network.o src/io/sound.o \
	src/level/event/bridge.o src/level/event/guardians.o \
	src/level/event/event.o src/level/event/eventframe.o \
	src/level/bullet.o src/level/demolevel.o src/level/level.o \
	src/level/levelframe.o src/level/levelload.o \
	src/menu/gamemenu.o src/menu/mainmenu.o src/menu/menu.o \
	src/menu/menuutil.o src/menu/setupmenu.o \
	src/player/bird.o src/player/player.o src/player/playerframe.o \
	src/main.o src/movable.o src/planet.o src/scene.o src/util.o \
	src/scale2x/getopt.o src/scale2x/pixel.o src/scale2x/scale2x.o \
	src/scale2x/scale3x.o src/scale2x/scalebit.o src/scale2x/simple2x.o


OpenJazz: $(objects)
	cc -Wall -o OpenJazz -lSDL -lstdc++ $(objects)

%.o: %.cpp
	cc -Wall -DUSE_SOCKETS -DSCALE -Isrc -O2 -c $< -o $@

clean:
	rm -f OpenJazz $(objects)
