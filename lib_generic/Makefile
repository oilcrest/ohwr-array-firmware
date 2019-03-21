# Makefile for lib_generic

ifeq (,$(BUILDDIR))

all:
	@echo "Must be invoked from top directory"
	@exit 1

else

include target_config.mk

COBJS-$(CONFIG_LIBGENERIC_STRICMP)    += $(BUILDDIR)/lib_generic/astring_stricmp.o
COBJS-$(CONFIG_LIBGENERIC_STRICMP_P)  += $(BUILDDIR)/lib_generic/astring_stricmp_P.o
COBJS-$(CONFIG_LIBGENERIC_STRNICMP)   += $(BUILDDIR)/lib_generic/astring_strnicmp.o
COBJS-$(CONFIG_LIBGENERIC_STRNICMP_P) += $(BUILDDIR)/lib_generic/astring_strnicmp_P.o
COBJS-y                               += $(BUILDDIR)/lib_generic/cmdarg.o
COBJS-y                               += $(BUILDDIR)/lib_generic/cmdproc.o
COBJS-y                               += $(BUILDDIR)/lib_generic/crc16.o
COBJS-y                               += $(BUILDDIR)/lib_generic/editor.o
COBJS-y                               += $(BUILDDIR)/lib_generic/fifo.o
COBJS-y                               += $(BUILDDIR)/lib_generic/fifostream.o
COBJS-y                               += $(BUILDDIR)/lib_generic/printf.o
COBJS-y                               += $(BUILDDIR)/lib_generic/stream.o
COBJS-y                               += $(BUILDDIR)/lib_generic/vt100.o

OBJS = $(COBJS-y)

DEPS = $(COBJS-y:.o=.c.dep)

all: $(BUILDDIR)/lib_generic/libgeneric.a

$(BUILDDIR)/lib_generic/libgeneric.a: $(OBJS)
	@echo "AR   $@"
	@$(AR) $(ARFLAGS) $@ $^

-include $(DEPS)

endif # BUILDDIR
	
.PHONY: all

.DELETE_ON_ERROR:

