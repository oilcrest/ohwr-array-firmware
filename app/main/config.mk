CONFIG_MULTITASKING = UCOS_II

APP_COBJS-y += $(BUILDDIR)/app/main/main.o
APP_COBJS-y += $(BUILDDIR)/app/main/stack_usage.o
APP_COBJS-y += $(BUILDDIR)/app/main/cli.o
APP_COBJS-y += $(BUILDDIR)/app/main/sys_info.o
APP_COBJS-y += $(BUILDDIR)/app/main/cmd_sys.o
APP_COBJS-y += $(BUILDDIR)/app/main/swmatrix.o
APP_COBJS-y += $(BUILDDIR)/app/main/ui.o

DEFINES += -DSYS_INFO_BUILD_REVISION=\""$(shell  git rev-parse HEAD)"\"
DEFINES += -DSYS_INFO_BUILD_DATE=\""$(shell date)"\"
DEFINES += -DSYS_INFO_BUILD_USER=\""$(shell whoami)"\"
DEFINES += -DSYS_INFO_BUILD_MACHINE=\""$(shell uname -n)"\"

# definitions for programming target device

EXECFILE = $(BUILDDIR)/progfile.elf
HEXFILE = $(BUILDDIR)/progfile.hex

MCU = atxmega128a1
STK500 = /cygdrive/c/Program\ Files\ \(x86\)/Atmel/AVR\ Tools/STK500/Stk500.exe
AVRDUDE = avrdude
WINPROG_FILE = $(EXECFILE)
WINPROG = $(STK500) -e -ip$< -pa -va -d$(MCU) -cUSB -md
PROG_FILE = $(HEXFILE)
PROG_AVRISP2 = $(AVRDUDE) -P usb -c avrisp2 -p $(MCU) -U flash:w:$<
PROG_AVR911 = $(AVRDUDE) -P /dev/ttyUSB0 -B 115200 -c avr911 -p $(MCU) -U flash:w:$< 

