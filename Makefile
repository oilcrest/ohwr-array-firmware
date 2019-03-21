# Makefile for firmware
# 
# Author: Adrian Matoga, AGH-UST, Cracow
# Author: Szymon Kulis, AGH-UST, Cracow | CERN, Geneva
#

# setup tools for this makefile and submakefiles

RM = rm -rf
MKDIR = mkdir -p
MAKE := $(MAKE) --no-print-directory

export RM MKDIR MAKE

ifneq (genconfig.mk,$(wildcard genconfig.mk))

# exit with error if configuration hasn't been generated yet

preprocess all prog winprog:
	@echo "Build not configured - see README"
	@exit 1

else			# genconfig.mk

# main target to build

all: progfiles

# include generated configuration (should set CROSS_COMPILE, CPU, SOC, BOARD and APP)

include genconfig.mk                  

export TARGET
export CROSS_COMPILE
export CPU
export SOC
export BOARD
export APP

# setup toolchain and flags

AS      = $(CROSS_COMPILE)as
LD      = $(CROSS_COMPILE)gcc
CC      = $(CROSS_COMPILE)gcc
CPP     = $(CROSS_COMPILE)cpp
AR      = $(CROSS_COMPILE)ar
NM      = $(CROSS_COMPILE)nm
STRIP	= $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

CPPFLAGS =
ASFLAGS  =
CFLAGS   = -Wall -std=gnu99 -Wno-deprecated-declarations -D__PROG_TYPES_COMPAT__
LDFLAGS  = 
ARFLAGS  = cr

export CPP AS CC LD AR NM STRIP OBJCOPY OBJDUMP
export CPPFLAGS ASFLAGS CFLAGS LDFLAGS ARFLAGS

BUILDDIR = build/$(TARGET)
export BUILDDIR

INCLDIRS :=
DEFINES :=

include target_config.mk              # include all target specific configuration files

CPPFLAGS += $(addprefix -I,$(INCLDIRS))
CPPFLAGS += $(DEFINES)

# initialize lists of objects and libraries to build

COBJS-y =
SOBJS-y =
LIBS-y  =

# setup libraries
# order is important - if x depends on y, y should be placed below x.

LIBS-$(CONFIG_DRIVERS) += $(BUILDDIR)/drivers/drivers.a
LIBS-y                 += $(BUILDDIR)/lib_generic/libgeneric.a

LIBS = $(LIBS-y)

# setup objects

COBJS-y += $(APP_COBJS-y)
SOBJS-y += $(APP_SOBJS-y)

OBJS = $(COBJS-y) $(SOBJS-y)
DEPS = $(COBJS-y:.o=.c.dep) $(SOBJS-y:.o=.s.dep)

# rules for building program files

PROGFILES = $(EXECFILE) $(HEXFILE)

progfiles: $(PROGFILES)

ifdef EXECFILE
$(EXECFILE): $(OBJS) $(LIBS)
	@echo "LD   $@"
	@$(LD) $^ -o $@ $(LDFLAGS)
endif

ifdef HEXFILE
$(HEXFILE): $(EXECFILE)
	@echo "HEX  $@"
	@$(OBJCOPY) -j .text -j .data -O ihex  $^ $@
endif

# preprocess sources

ALL_SOURCES := $(shell find . -name preprocessed -prune -o \( \( -name *.c -or -name *.s \) -print \) )
ALL_PREPROCESSED := $(addprefix preprocessed/,$(ALL_SOURCES))

preprocessed/%.c: %.c
	@echo "CPP  $@"
	@$(MKDIR) $$(dirname "$@") && if ! $(CPP) $< $(CPPFLAGS) >$@; then echo "Warning: couldn't preprocess $<"; $(RM) $@; fi

preprocessed/%.s: %.s
	@echo "CPP  $@"
	@$(MKDIR) $$(dirname "$@") && if ! $(CPP) $< $(CPPFLAGS) >$@; then echo "Warning: couldn't preprocess $<"; $(RM) $@; fi

preprocess: $(ALL_PREPROCESSED)

# rules for building documentation

ifdef EXECFILE
disasm: disasm.txt

disasm.txt: $(EXECFILE)
	$(OBJDUMP) -d $< >$@
endif

doc:
	doxygen

# rules for building libraries

$(BUILDDIR)/lib_generic/libgeneric.a:
	@$(MAKE) -f lib_generic/Makefile

#$(BUILDDIR)/ucos/ucos.a:
#	@$(MAKE) -f ucos/Makefile

$(BUILDDIR)/drivers/drivers.a:
	@$(MAKE) -f drivers/Makefile

# auto generated dependencies

-include $(DEPS)

endif			# genconfig.mk

####################################################################################################

# target (application/board) configurations

default_config: unconfig
	@./mkconfig.sh main avr- avr atxmega128a1 swmatrix main

unconfig:
	@$(RM) genconfig.mk

####################################################################################################

# programming rules

ifdef WINPROG
winprog: $(WINPROG_FILE)
	$(WINPROG)
endif # WINPROG

#ifdef PROG
prog_avrisp2: $(PROG_FILE)
	$(PROG_AVRISP2)

prog_avr911: $(PROG_FILE)
	$(PROG_AVR911)

app/bootloader/Xmega_Bootloader.hex:
	cd  app/bootloader && make all

bootloader:app/bootloader/Xmega_Bootloader.hex

prog_bootloader: app/bootloader/Xmega_Bootloader.hex
	$(AVRDUDE) -P usb -c avrisp2 -p $(MCU) -U flash:w:$<

prog_write_fuses:
	$(AVRDUDE) -P usb -c avrisp2 -p $(MCU) -U fuse2:w:0xBF:m 

prog_read_fuses:
	$(AVRDUDE) -P usb -c avrisp2 -p $(MCU) -U fuse0:r:fuse0.hex:h -U fuse1:r:fuse1.hex:h -U fuse2:r:fuse2.hex:h 

prog_all: prog_avrisp2 prog_bootloader prog_write_fuses
#-U flash:w:$<

#endif # PROG

####################################################################################################

# 

clean:
	$(RM) $(BUILDDIR) preprocessed doc *.gcov disasm.txt

.PHONY: all clean preprocess unconfig $(LIBS) prog_read_fuses prog_bootloader prog_write_fuses prog_all

.DELETE_ON_ERROR:

