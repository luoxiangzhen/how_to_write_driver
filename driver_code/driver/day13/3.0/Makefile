MODNAME=s5pv210_adc_drv
CC=arm-linux-gcc
APP=s5pv210_adc_test

ifeq ($(PLATFORM),PC)
    KERNELDIR=/lib/modules/$(shell -uname -r)/build
else
    KERNELDIR=/opt/kernel		
endif

obj-m += $(MODNAME).o

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	$(CC) -o $(APP) $(APP).c
clean:
	rm -fr *.mod.* *.o *.ko .$(MODNAME).* .tmp* Module.* modules.* $(APP)
