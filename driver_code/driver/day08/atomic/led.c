#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>


struct led_resouce{
    int     gpio;
    char    *name;
};

// led设备资源信息
static struct led_resouce led_info[] = {
    [0] = {
        .gpio = S5PV210_GPC1(3),
        .name = "led1"
    },
    [1] = {
        .gpio = S5PV210_GPC1(4),
        .name = "led2"
    },
};

// 设备号
static dev_t dev;
// 字符设备
static struct cdev led_cdev;
// 设备类
static struct class *cls;

// 分配整型原子变量
static atomic_t open_cnt;

// A进程打开不关闭，B进程不允许打开
static int led_open(struct inode* inode, struct file* file){
    int i;

    if(!atomic_dec_and_test(&open_cnt)){
        atomic_inc(&open_cnt);
        // 打开设备失败
        return -EBUSY;
    }
    
    for(i=0; i<ARRAY_SIZE(led_info); ++i)
        gpio_set_value(led_info[i].gpio, 1);

    return 0;
}

// A进程关闭，B进程允许打开
static int led_close(struct inode* inode, struct file* file){
    int i;

    for(i=0; i<ARRAY_SIZE(led_info); ++i)
        gpio_set_value(led_info[i].gpio, 0);

    atomic_inc(&open_cnt);
    return 0;
}


static struct file_operations led_fops = {
    .owner   = THIS_MODULE,
    .open    = led_open,
    .release = led_close,
};


static int led_init(void){
    int i;

    // 分配设备号
    alloc_chrdev_region(&dev, 0, 1, "leds");
    // 初始化字符设备对象
    cdev_init(&led_cdev, &led_fops);
    // 注册字符设备对象
    cdev_add(&led_cdev, dev, 1);
    // 创建设备文件
    cls = class_create(THIS_MODULE, "LEDS");
    device_create(cls, NULL, dev, NULL, "leds");
    
    // 设置整型原子变量的值为1
    atomic_set(&open_cnt, 1);

    // 申请GPIO资源
    for(i=0; i<ARRAY_SIZE(led_info); ++i){
        gpio_request(led_info[i].gpio, led_info[i].name);
        gpio_direction_output(led_info[i].gpio, 0);
    }

    return 0;
}

static void led_exit(void){
    int i;

    // 删除设备文件
    device_destroy(cls, dev);
    class_destroy(cls);
    // 卸载字符设备对象
    cdev_del(&led_cdev);
    // 释放设备号
    unregister_chrdev_region(dev, 1);

    // 释放GPIO资源
    for(i=0; i<ARRAY_SIZE(led_info); ++i){
        gpio_set_value(led_info[i].gpio, 0);
        gpio_free(led_info[i].gpio);
    }
}


module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
