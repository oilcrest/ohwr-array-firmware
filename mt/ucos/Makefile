# Makefile for uC/OS-II
#
# Author: Adrian Matoga, AGH-UST Cracow
#
# Note: currently not used, as uC/OS-II objects
#       are linked directly with application
#       instead of building library first
#
# See also: top-level Makefile and ucos/config.mk

ifeq (,$(BUILDDIR))
all:
	@echo "Must be invoked from top directory"
	@exit 1
endif

UCOS_COBJS-y =
UCOS_SOBJS-y =

include toolchain_config.mk
include target_config.mk

UCOS_COBJS-y += $(BUILDDIR)/ucos/os_core.o
UCOS_COBJS-y += $(BUILDDIR)/ucos/os_flag.o
UCOS_COBJS-y += $(BUILDDIR)/ucos/os_mbox.o
UCOS_COBJS-y += $(BUILDDIR)/ucos/os_mem.o
UCOS_COBJS-y += $(BUILDDIR)/ucos/os_mutex.o
UCOS_COBJS-y += $(BUILDDIR)/ucos/os_q.o
UCOS_COBJS-y += $(BUILDDIR)/ucos/os_sem.o
UCOS_COBJS-y += $(BUILDDIR)/ucos/os_task.o
UCOS_COBJS-y += $(BUILDDIR)/ucos/os_time.o
UCOS_COBJS-y += $(BUILDDIR)/ucos/os_tmr.o

OBJS = $(UCOS_COBJS-y) $(UCOS_SOBJS-y)

DEPS = $(UCOS_COBJS-y:.o=.c.dep) $(UCOS_SOBJS-y:.o=.s.dep)

all: $(BUILDDIR)/ucos/ucos.a

$(BUILDDIR)/ucos/ucos.a: $(OBJS)
	@echo "AR   $@"
	@$(AR) $(ARFLAGS) $@ $^

-include $(DEPS)

.PHONY: all

.DELETE_ON_ERROR:
