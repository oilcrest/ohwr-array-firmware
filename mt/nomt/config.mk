# target dependent configuration for no-multitasking may be found in various directories

ifdef CPU
ifeq (cpu/$(CPU)/nomt,$(wildcard cpu/$(CPU)/nomt))
INCLDIRS += cpu/$(CPU)/nomt
-include cpu/$(CPU)/nomt/config.mk
endif
ifdef SOC
ifeq (cpu/$(CPU)/$(SOC)/nomt,$(wildcard cpu/$(CPU)/$(SOC)/nomt))
INCLDIRS += cpu/$(CPU)/$(SOC)/nomt
-include cpu/$(CPU)/$(SOC)/nomt/config.mk
endif
endif # SOC
endif # CPU

ifdef BOARD
ifeq (board/$(BOARD)/nomt,$(wildcard board/$(BOARD)/nomt))
INCLDIRS += board/$(BOARD)/nomt
-include board/$(BOARD)/nomt/config.mk
endif
endif # BOARD

ifdef APP
ifeq (app/$(APP)/nomt,$(wildcard app/$(APP)/nomt))
INCLDIRS += app/$(APP)/nomt
-include app/$(APP)/nomt/config.mk
endif
endif # APP

