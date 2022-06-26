# OpenJazz PSP Port Makefile

TARGET = OpenJazz
include ../../openjazz.mk
OBJS:=$(patsubst %,../../%,$(OBJS))

# disable scaling
OBJS:=$(filter-out ../../ext/scale2x/%.o,$(OBJS))
# add psp specific source
OBJS += ../../src/platforms/psp.o

PSPSDK=$(shell psp-config --pspsdk-path)
PSPPREFIX=$(shell psp-config --psp-prefix)

CXXFLAGS = -g -O2 -G0 -Wall -DPSP
INCDIR = $(PSPPREFIX)/include/SDL \
        ../../ext/psmplug ../../ext/miniz ../../ext/argparse ../../src
LIBS = -lSDL -lGL -lm -lstdc++ \
       -lpspdebug -lpspgu -lpspctrl -lpspge -lpspdisplay -lpsphprm \
       -lpspvfpu -lpsprtc -lpspaudio -lpsputility -lpspirkeyb -lpsppower

BUILD_PRX = 1
#ENCRYPT = 1
#PSP_FW_VERSION = 330

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Jazz Jackrabbit (OpenJazz)
PSP_EBOOT_ICON = "icon.png"
#PSP_EBOOT_PIC1 = "background.png" # (480x272)

include $(PSPSDK)/lib/build.mak

# silent building
%.o: %.cpp
	@echo "[CXX] $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@
