# OpenJazz makefile

objects = bird.o bonus.o bullet.o events.o file.o font.o game.o level.o \
	levelload.o main.o menu.o palette.o planet.o player.o scene.o sound.o \
	sprite.o util.o


OpenJazz: $(objects)
	cc -Wall -o OpenJazz -lSDL -lstdc++ $(objects)

bird.o: level.h
bonus.o: bonus.h file.h
bullet.o: game.h level.h palette.h
events.o: level.h
file.o: file.h
font.o: font.h
game.o: font.h game.h level.h menu.h palette.h
level.o: font.h game.h level.h menu.h palette.h
levelload.o: font.h game.h level.h palette.h
main.o: font.h game.h level.h menu.h palette.h
menu.o: font.h game.h level.h menu.h
palette.o: level.h palette.h
planet.o: file.h planet.h
player.o: font.h game.h level.h palette.h
scene.o: file.h scene.h
sound.o: file.h
sprite.o: level.h
util.o: file.h

bird.h: OpenJazz.h
bonus.h: OpenJazz.h
bullet.h: events.h player.h
events.h: player.h
file.h: OpenJazz.h
font.h: file.h
game.h: file.h
level.h: bullet.h events.h scene.h sprite.h
menu.h: OpenJazz.h
palette.h: OpenJazz.h
planet.h: OpenJazz.h
player.h: bird.h
scene.h: OpenJazz.h
sprite.h: OpenJazz.h

clean:
	rm -f OpenJazz $(objects)
