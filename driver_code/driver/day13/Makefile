obj-m += led_dev.o led_drv.o # 编译为模块

KDIR = /opt/kernel	 # 制定内核源码路径
DDIR = /opt/rootfs/lib/
SDIR = /opt/lib/*

# 编译模块
all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
# -C 指定到某一目录编译

# 安装模块
install:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) \
	    modules_install INSTALL_MOD_PATH=/opt
	cp -rfd $(SDIR) $(DDIR)
# INSTALL_MOD_PATH指定安装目录在/opt，会生成一个lib目录

clean:
	rm -rf *.ko *.o *.mod.* *.order \
	    *.sym* *.cmd *.swp .*.cmd .tmp*
