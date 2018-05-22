# OpenJazz makefile
include openjazz.mk

# Sane defaults
CXX ?= g++
CXXFLAGS ?= -g -Wall -O2

CXXFLAGS += -DSCALE

# Network support
CXXFLAGS += -DUSE_SOCKETS
# Needed under Windows
#LIBS += -lws2_32

# SDL
CXXFLAGS += $(shell sdl-config --cflags)
LIBS += $(shell sdl-config --libs)

# music library: modplug, xmp
MUSICLIB ?= modplug
MUSICLIB_CFLAGS ?= $(shell pkg-config --silence-errors --cflags lib$(MUSICLIB))
MUSICLIB_LIBS ?= $(shell pkg-config --silence-errors --libs lib$(MUSICLIB))
ifneq ($(MUSICLIB_LIBS),)
	CXXFLAGS += -DUSE_$(MUSICLIB) $(MUSICLIB_CFLAGS)
	LIBS += $(MUSICLIB_LIBS)
endif

LIBS += -lm -lz

OpenJazz: $(OBJS)
	$(CXX) -o OpenJazz $(LDFLAGS) $(OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -Isrc -c $< -o $@

clean:
	rm -f OpenJazz $(OBJS)
