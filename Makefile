# OpenJazz makefile
include openjazz.mk

# Sane defaults
CXX ?= g++
CXXFLAGS ?= -g -Wall -O2
CPPFLAGS = -Isrc -DSCALE -Iext/scale2x -Iext/psmplug -Iext/miniz

# Network support
CXXFLAGS += -DUSE_SOCKETS
# Needed under Windows
#LIBS += -lws2_32

# SDL
CXXFLAGS += $(shell sdl-config --cflags)
LIBS += $(shell sdl-config --libs)

LIBS += -lm -lz

OpenJazz: $(OBJS)
	$(CXX) -o OpenJazz $(LDFLAGS) $(OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f OpenJazz $(OBJS)
