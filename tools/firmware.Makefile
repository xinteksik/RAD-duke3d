# Private RAD DUKE3D link, derived from the frozen RAD-Wolf firmware rules.
REPO ?= /tmp/radduke-work
BASE := $(REPO)/upstream/rad-baseline
STDLIB_ROOT := $(BASE)/upstream/circle-stdlib
RAD := $(BASE)/upstream/RAD-Doom/Source
RW := $(REPO)/src
CORE := $(REPO)/build/menu-arm64
CIRCLEHOME := $(STDLIB_ROOT)/libs/circle
NEWLIBDIR := $(STDLIB_ROOT)/install/aarch64-none-circle
include $(STDLIB_ROOT)/Config.mk
AARCH := 64
RASPPI := 3
PREFIX64 := aarch64-none-elf-
TARGET := kernel8
RAD_NAMES := config rad_reu lowlevel_arm64 gpio_defs helpers lowlevel_dma mempool
RAD_OBJS := $(addsuffix .o,$(RAD_NAMES))
GAME_OBJS := $(addprefix core/,$(notdir $(wildcard $(CORE)/*.o)))
OBJS := radduke_kernel.o radduke_rad_video.o radduke_rad_hijack.o radduke_rad_bindings.o radduke_fs.o radduke_newlib_io.o $(RAD_OBJS) $(GAME_OBJS)
INCLUDE += -I$(NEWLIBDIR)/include -I$(STDLIB_ROOT)/include -I$(RW) -I$(BASE)/src -I$(RAD) -I$(RAD)/../include -I$(STDLIB_ROOT)/build/circle-newlib/aarch64-none-circle/libgloss/circle -I$(STDLIB_ROOT)/libs/circle-newlib/libgloss/circle
LIBS := $(NEWLIBDIR)/lib/libm.a $(NEWLIBDIR)/lib/libc.a $(NEWLIBDIR)/lib/libcirclenewlib.a \
 $(CIRCLEHOME)/addon/SDCard/libsdcard.a $(CIRCLEHOME)/lib/usb/libusb.a \
 $(CIRCLEHOME)/lib/input/libinput.a $(CIRCLEHOME)/addon/fatfs/libfatfs.a \
 $(CIRCLEHOME)/lib/fs/libfs.a $(CIRCLEHOME)/lib/net/libnet.a \
 $(CIRCLEHOME)/lib/sched/libsched.a $(CIRCLEHOME)/lib/libcircle.a
include $(CIRCLEHOME)/Rules.mk
CFLAGS += -Ofast -g0 -Wno-unused-variable -fexceptions -DRADWOLF_BAREMETAL $(RADDUKE_EXTRA_FLAGS)
CPPFLAGS += $(CFLAGS) -include $(CIRCLEHOME)/include/assert.h
$(shell mkdir -p core)
radduke_kernel.o radduke_rad_hijack.o radduke_rad_bindings.o radduke_fs.o: %.o: $(RW)/%.cpp
	$(CPP) $(CPPFLAGS) -c -o $@ $<
radduke_newlib_io.o: $(RW)/radduke_newlib_io.cpp
	$(CPP) -I$(STDLIB_ROOT)/build/circle-newlib/aarch64-none-circle/libgloss/circle $(CPPFLAGS) -c -o $@ $<
radduke_rad_video.o: $(RW)/radduke_rad_video.c
	$(CC) $(CFLAGS) -std=gnu99 -c -o $@ $<
$(RAD_OBJS): %.o: $(RAD)/%.cpp
	$(CPP) $(CPPFLAGS) -c -o $@ $<
core/%.o: $(CORE)/%.o
	cp $< $@
all: $(TARGET).img
-include $(DEPS)
