#---------------------------------------------------------------------------------
.SUFFIXES:

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

#---------------------------------------------------------------------------------
TARGET		:= OpenJazz
BUILD		:= build
INCLUDES	:= src ext/miniz ext/psmplug
APP_TITLE	:= OpenJazz
APP_DESCRIPTION	:=
APP_AUTHOR	:= AlisterT & carstene1ns
ICON		:= ../unix/OpenJazz.png

# include all source files
include $(TOPDIR)/../openjazz.mk
# disable scaling
OBJS := $(filter-out ext/scale2x/%.o,$(OBJS))

#---------------------------------------------------------------------------------
ARCH	:= -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CFLAGS	:= -g -Wall -O2 -mword-relocations \
           -fomit-frame-pointer -ffunction-sections -fdata-sections \
           $(ARCH)
CFLAGS	+= $(INCLUDE) -DARM11 -D_3DS -DDATAPATH=\"sdmc:/3ds/OpenJazz/\"
CXXFLAGS:= $(CFLAGS) -frtti -fexceptions
ASFLAGS	:= -g $(ARCH)
LDFLAGS	=  -specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)
LIBS	:= -lSDL -lcitro3d -lctru -lm
LIBDIRS	:= $(CTRULIB) $(PORTLIBS)

#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT	:= $(CURDIR)/$(TARGET)
export TOPDIR	:= $(CURDIR)
export VPATH	:= $(sort $(foreach obj,$(OBJS),$(CURDIR)/../$(dir $(obj))))
export DEPSDIR	:= $(CURDIR)/$(BUILD)
export LD	:= $(CXX)
export OFILES	:= $(foreach obj,$(OBJS),$(notdir $(obj)))
export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(CURDIR)/../$(dir)) \
                   $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                   -I$(PORTLIBS)/include/SDL \
                   -I$(CURDIR)/$(BUILD)
export LIBPATHS	:= $(foreach dir,$(LIBDIRS),-L$(dir)/lib)
export APP_ICON := $(TOPDIR)/$(ICON)
export _3DSXFLAGS += --smdh=$(CURDIR)/$(TARGET).smdh

.PHONY: all clean

all: $(BUILD) $(DEPSDIR)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

$(BUILD):
	@mkdir -p $@

ifneq ($(DEPSDIR),$(BUILD))
$(DEPSDIR):
	@mkdir -p $@
endif

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).3dsx $(OUTPUT).smdh $(TARGET).elf

#---------------------------------------------------------------------------------
else

DEPENDS		:= $(OFILES:.o=.d)

$(OUTPUT).3dsx	:  $(OUTPUT).elf $(OUTPUT).smdh

$(OUTPUT).elf	:  $(OFILES)

-include $(DEPSDIR)/*.d

endif
#---------------------------------------------------------------------------------------
