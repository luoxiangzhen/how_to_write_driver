obj-m += led_drv.o
KDIR=/opt/kernel
all:
	make -C $(KDIR) SUBDIRS=$(PWD) modules
clean:
	rm -fr *.mod.* *.ko *.o .tmp* module.* Module* modules* .*.cmd 
