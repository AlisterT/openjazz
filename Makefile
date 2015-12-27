# OpenJazz makefile

include openjazz.mk

OpenJazz: $(OBJS)
	cc -Wall -o OpenJazz -lSDL -lstdc++ -lz $(OBJS)

%.o: %.cpp
	cc -Wall -DUSE_SOCKETS -DSCALE -Isrc -O2 -c $< -o $@

clean:
	rm -f OpenJazz $(OBJS)
