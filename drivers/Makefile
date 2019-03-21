# Makefile for drivers

ifeq (,$(BUILDDIR))

all:
	@echo "Must be invoked from top directory"
	@exit 1

else

include target_config.mk

COBJS-y =
SOBJS-y =

###########################################################################

# platform specific implementations of common drivers 

# atxmega

COBJS-$(CONFIG_DRIVERS_XMEGA) += $(BUILDDIR)/drivers/xmega/xmega_usart.o
COBJS-$(CONFIG_DRIVERS_XMEGA) += $(BUILDDIR)/drivers/xmega/config_file.o

###########################################################################

# platform specific drivers

# atxmega

COBJS-$(CONFIG_DRIVERS_XMEGA) += $(BUILDDIR)/drivers/xmega/adc_driver.o
COBJS-$(CONFIG_DRIVERS_XMEGA) += $(BUILDDIR)/drivers/xmega/clksys_driver.o
COBJS-$(CONFIG_DRIVERS_XMEGA) += $(BUILDDIR)/drivers/xmega/clksys_getfreq.o
COBJS-$(CONFIG_DRIVERS_XMEGA) += $(BUILDDIR)/drivers/xmega/dac_driver.o
COBJS-$(CONFIG_DRIVERS_XMEGA) += $(BUILDDIR)/drivers/xmega/dma_driver.o
COBJS-$(CONFIG_DRIVERS_XMEGA) += $(BUILDDIR)/drivers/xmega/dma_alloc.o
#COBJS-$(CONFIG_DRIVERS_XMEGA) += $(BUILDDIR)/drivers/xmega/ebi_driver.o
COBJS-$(CONFIG_DRIVERS_XMEGA) += $(BUILDDIR)/drivers/xmega/eeprom_driver.o
COBJS-$(CONFIG_DRIVERS_XMEGA) += $(BUILDDIR)/drivers/xmega/pmic_driver.o
COBJS-$(CONFIG_DRIVERS_XMEGA) += $(BUILDDIR)/drivers/xmega/port_driver.o
COBJS-$(CONFIG_DRIVERS_XMEGA) += $(BUILDDIR)/drivers/xmega/sp_driver.o
COBJS-$(CONFIG_DRIVERS_XMEGA) += $(BUILDDIR)/drivers/xmega/system_driver.o
COBJS-$(CONFIG_DRIVERS_XMEGA) += $(BUILDDIR)/drivers/xmega/TC_driver.o
COBJS-$(CONFIG_DRIVERS_XMEGA) += $(BUILDDIR)/drivers/xmega/TWI_master.o


###########################################################################

# driver supporting files

COBJS-y += $(BUILDDIR)/drivers/serialstream.o

###########################################################################

OBJS = $(COBJS-y)

DEPS = $(COBJS-y:.o=.c.dep)

all: $(BUILDDIR)/drivers/drivers.a

$(BUILDDIR)/drivers/drivers.a: $(OBJS)
	@echo "AR   $@"
	@$(AR) $(ARFLAGS) $@ $^

-include $(DEPS)

endif # BUILDDIR

.PHONY: all

.DELETE_ON_ERROR:

