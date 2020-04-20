# include all target specific configuration files

ifdef APP
-include app/$(APP)/config.mk         # include application specific rules
INCLDIRS += app/$(APP)
endif # APP

ifdef BOARD
-include board/$(BOARD)/config.mk     # include board specific rules
INCLDIRS += board/$(BOARD)
endif # BOARD

ifdef CPU
-include cpu/$(CPU)/config.mk         # include CPU specific rules
INCLDIRS += cpu/$(CPU)
ifdef SOC
-include cpu/$(CPU)/$(SOC)/config.mk  # include SoC/MCU specific rules
INCLDIRS += cpu/$(CPU)/$(SOC)
endif # SOC
endif #CPU

ifeq (UCOS_II,$(CONFIG_MULTITASKING))
include mt/ucos/config.mk
else
include mt/nomt/config.mk
endif

ifdef BOARD
ifeq (board/$(BOARD)/link.x,$(wildcard board/$(BOARD)/link.x))
LDSCRIPT = board/$(BOARD)/link.x
LDFLAGS += -Wl,--script=$(LDSCRIPT)
endif
endif #BOARD

INCLDIRS += board
INCLDIRS += lib_generic
INCLDIRS += drivers

# rules for building objects

$(BUILDDIR)/%.dep: %
	@echo "DEP  $<"
	@$(MKDIR) $$(dirname "$@") && $(CPP) -MM $< $(CPPFLAGS) | sed "s,\(.*\)\.o[ :]*,$(@D)/\1.o $@ : ,g" > $@

$(BUILDDIR)/%.o: %.c
	@echo "CC   $@"
	@$(CC) $< -c -o $@ $(CPPFLAGS) $(CFLAGS)

$(BUILDDIR)/%.o: %.s
	@echo "AS   $@"
	@$(CPP) $< $(CPPFLAGS) | $(AS) $(ASFLAGS) -o $@

