obj-m += helloworld.o

KDIR = /opt/kernel

all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
install:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules_install \
	    INSTALL_MOD_PATH=/opt

clean:
	rm -rf *.ko *.o .tmp* module.* \
	    Module* modules* .*.cmd *.mod.*
