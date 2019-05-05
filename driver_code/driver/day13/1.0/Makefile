obj-m += btn_drv.o btn_dev.o
KDIR=/opt/kernel
all:
	make -C $(KDIR) SUBDIRS=$(PWD) modules
clean:
	rm -fr *.mod.* *.ko *.o .tmp* module.* Module* modules* .*.cmd 
