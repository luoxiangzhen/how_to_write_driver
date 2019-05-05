// 平台无关的头文件
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
// 跟平台硬件相关的头文件
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>

static int major;   // 主设备号
module_param(major, int, 0);

struct led_resource{
    int     gpio;
    char    *name;
};

static struct led_resource led_info[] = {
    [0] = {
        .gpio = S5PV210_GPC1(3),
        .name = "LED1"
    },
    [1] = {
        .gpio = S5PV210_GPC1(4),
        .name = "LED2"
    },
};


// 底层操作方法
// app:open->swi->sys_open->led_open
static int led_open(struct inode *inode, struct file *file){
    printk("%s\n", __func__);
    int i;
    for(i=0; i<ARRAY_SIZE(led_info); ++i)
        gpio_set_value(led_info[i].gpio, 1);

    return 0; // 成功返回0，失败返回负值
}


// app:close->swi->sys_close->led_close
static int led_close(struct inode *inode, struct file *file){
    printk("%s\n", __func__);
    int i;
    for(i=0; i<ARRAY_SIZE(led_info); ++i)
        gpio_set_value(led_info[i].gpio, 0);

    return 0; // 成功返回0，失败返回负值
}


// 分配一个字符设备
static struct cdev led_cdev;
// 分配初始化驱动操作硬件的方法
static struct file_operations led_fops = {
    .owner      = THIS_MODULE, // 模块的所属者
    .open       = led_open,    // 打开设备
    .release    = led_close,   // 关闭设备
};

static int led_init(void){
    // 设备号
    dev_t dev;

    if(major){
        // 静态申请设备号
        // 构造设备号
        dev = MKDEV(major, 0);
        // 注册设备
        register_chrdev_region(dev, 1, "led");
    }else{
        // 动态申请设备号
        // dev - 设备号
        // 0 - 希望起始的次设备号从0开始
        // 1 - 设备的个数为1个
        // “tarena2” - 设备名称
        alloc_chrdev_region(&dev, 0, 1, "led");
        // 提取主设备号
        major = MAJOR(dev);
    }

    // 初始化字符设备对象
    // 初始化完成后led_cdev就有了操作硬件的方法
    // led_cdev.ops = &led_fops;
    cdev_init(&led_cdev, &led_fops);
    
    // 向内核注册添加安装字符设备对象
    // 一旦添加成功内核就有了真正的字符设备驱动
    // led_cdev - 分配字符设备对象
    // dev - 设备号
    // 1 - 设备个数
    cdev_add(&led_cdev, dev, 1);

    int i;
    for(i=0; i<ARRAY_SIZE(led_info); ++i){
        // 申请GPIO资源
        gpio_request(led_info[i].gpio, led_info[i].name);
        // 配置为输出口，默认输出为0
        gpio_direction_output(led_info[i].gpio, 0);
    }

    return 0;
}

static void led_exit(void){
    // 卸载字符设备对象
    // 一旦删除，内核就不会存在该字符设备驱动
    cdev_del(&led_cdev);

    // 释放GPIO资源
    int i;
    for(i=0; i<ARRAY_SIZE(led_info); ++i){
        gpio_direction_output(led_info[i].gpio, 0);
        gpio_free(led_info[i].gpio);
    }

    // 获取已经申请的设备号
    dev_t dev = MKDEV(major, 0);
    // 释放设备号
    unregister_chrdev_region(dev, 1);
}


module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
