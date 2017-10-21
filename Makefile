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

# modplug
MODPLUG_CFLAGS ?= $(shell pkg-config --silence-errors --cflags libmodplug)
MODPLUG_LIBS ?= $(shell pkg-config --silence-errors --libs libmodplug)
ifneq ($(MODPLUG_LIBS),)
	CXXFLAGS += -DUSE_MODPLUG $(MODPLUG_CFLAGS)
	LIBS += $(MODPLUG_LIBS)
endif

# xmp
XMP_CFLAGS ?= $(shell pkg-config --silence-errors --cflags libxmp)
XMP_LIBS ?= $(shell pkg-config --silence-errors --libs libxmp)
ifneq ($(XMP_LIBS),)
	CXXFLAGS += -DUSE_XMP $(XMP_CFLAGS)
	LIBS += $(XMP_LIBS)
endif

LIBS += -lm

OpenJazz: $(OBJS)
	$(CXX) $(LDFLAGS) -lz $(LIBS) $(OBJS) -o OpenJazz

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -Isrc -c $< -o $@

clean:
	rm -f OpenJazz $(OBJS)
