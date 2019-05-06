export BUILD_DIR=$(PWD)
export STAGING_DIR=$(BUILD_DIR)

export MAKECMD=make ARCH=mips CROSS_COMPILE=mipsel-openwrt-linux-
export PATH:=/home/nolan/pro/nolan/how_to_write_driver/driver/toolchain/openwrt-toolchain-ramips-mt7620_gcc-7.3.0_musl.Linux-x86_64/toolchain-mipsel_24kc_gcc-7.3.0_musl/bin:$(PATH)

export KERNELDIR:=/home/nolan/work/GS/lede_projects/build_dir/target-mipsel_24kc_musl/linux-ramips_mt7620/linux-4.14.34
obj-m := hello_world.o

all:
	$(MAKECMD) -C $(KERNELDIR) M=$(BUILD_DIR) modules
#	@cp watchdog.ko ~/work/
#	$(MAKECMD) clean

.PHONY :clean
clean:
	@rm -rf *.o *.ko .*.cmd *.mod.c modules.order Module.symvers .tmp_versions
