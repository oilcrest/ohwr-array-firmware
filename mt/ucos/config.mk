# target dependent configuration for uC/OS-II may be found in various directories

INCLDIRS += mt/ucos 
INCLDIRS += mt/ucos/orig

ifdef CPU
ifeq (cpu/$(CPU)/ucos,$(wildcard cpu/$(CPU)/ucos))
INCLDIRS += cpu/$(CPU)/ucos
-include cpu/$(CPU)/ucos/config.mk
endif
ifdef SOC
ifeq (cpu/$(CPU)/$(SOC)/ucos,$(wildcard cpu/$(CPU)/$(SOC)/ucos))
INCLDIRS += cpu/$(CPU)/$(SOC)/ucos
-include cpu/$(CPU)/$(SOC)/ucos/config.mk
endif
endif # SOC
endif # CPU

ifdef BOARD
ifeq (board/$(BOARD)/ucos,$(wildcard board/$(BOARD)/ucos))
INCLDIRS += board/$(BOARD)/ucos
-include board/$(BOARD)/ucos/config.mk
endif
endif # BOARD

ifdef APP
ifeq (app/$(APP)/ucos,$(wildcard app/$(APP)/ucos))
INCLDIRS += app/$(APP)/ucos
-include app/$(APP)/ucos/config.mk
endif
endif # APP

APP_COBJS-y += $(BUILDDIR)/mt/ucos/orig/os_core.o
APP_COBJS-y += $(BUILDDIR)/mt/ucos/orig/os_flag.o
APP_COBJS-y += $(BUILDDIR)/mt/ucos/orig/os_mbox.o
APP_COBJS-y += $(BUILDDIR)/mt/ucos/orig/os_mem.o
APP_COBJS-y += $(BUILDDIR)/mt/ucos/orig/os_mutex.o
APP_COBJS-y += $(BUILDDIR)/mt/ucos/orig/os_q.o
APP_COBJS-y += $(BUILDDIR)/mt/ucos/orig/os_sem.o
APP_COBJS-y += $(BUILDDIR)/mt/ucos/orig/os_task.o
APP_COBJS-y += $(BUILDDIR)/mt/ucos/orig/os_time.o
APP_COBJS-y += $(BUILDDIR)/mt/ucos/orig/os_tmr.o

