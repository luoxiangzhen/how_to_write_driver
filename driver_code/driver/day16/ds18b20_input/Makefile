# makefile for kernel module

MODNAME = ds18b20_drv
CROSS_COMPILE = arm-linux-
TESTPRE	= ds18b20_test

ifeq ($(PLATFORM), PC)
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
else
CC = $(CROSS_COMPILE)gcc
KERNELDIR ?= /opt/kernel
endif

obj-m := $(MODNAME).o

default: test driver

test:
	@echo "Compiling test program..."
	$(CC) $(TESTPRE).c -o $(TESTPRE)
	@echo

driver:
	@echo "Compiling device driver..."
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	@echo

clean:
	@echo "Cleanup test program..."
	@rm -f $(TESTPRE) $(MODNAME)
	@echo
	@echo "Cleanup device driver..."
	@rm -rf *.o *.ko *.mod.* .$(MODNAME).* .tmp* module* Module*
	@echo

