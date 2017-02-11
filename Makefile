# OpenJazz makefile
prefix=/usr/local

include openjazz.mk

CXXFLAGS += -g -Wall -O2 -DSCALE
CXXFLAGS += -DUSE_SOCKETS

# SDL flags
CXXFLAGS+=`sdl-config --cflags`
LDFLAGS+=`sdl-config --libs`

# Enable modplug music
CXXFLAGS += -DUSE_MODPLUG `pkg-config --cflags libmodplug`
LDFLAGS += `pkg-config --libs libmodplug`

LDFLAGS += -lm

OpenJazz: $(OBJS)
	cc $(CXXFLAGS) -o OpenJazz $(LDFLAGS) -lstdc++ -lz $(OBJS)

%.o: %.cpp
	cc $(CXXFLAGS) -Isrc -c $< -o $@

clean:
	rm -f OpenJazz $(OBJS)

install: OpenJazz
	install -m 0755 OpenJazz $(prefix)/bin
	install -m 0644 openjazz.000 $(prefix)/bin

.PHONY: install
