#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

static int major;   // 主设备号
module_param(major, int, 0);

static int alloc_chrdev_init(void){
    // 设备号
    dev_t dev;

    if(major){
        // 静态申请设备号
        // 构造设备号
        dev = MKDEV(major, 0);
        // 注册设备
        register_chrdev_region(dev, 1, "tarena1");
    }else{
        // 动态申请设备号
        // dev - 设备号
        // 0 - 希望起始的次设备号从0开始
        // 1 - 设备的个数为1个
        // “tarena2” - 设备名称
        alloc_chrdev_region(&dev, 0, 1, "tarena2");
        // 提取主设备号
        major = MAJOR(dev);
    }

    return 0;
}

static void alloc_chrdev_exit(void){
    // 获取已经申请的设备号
    dev_t dev = MKDEV(major, 0);
    // 释放设备号
    unregister_chrdev_region(dev, 1);
}

module_init(alloc_chrdev_init);
module_exit(alloc_chrdev_exit);

MODULE_LICENSE("GPL");
