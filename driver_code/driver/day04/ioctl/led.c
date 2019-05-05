#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>


#define     CMD_LED_ON      0x100001    // 开灯
#define     CMD_LED_OFF     0x100002    // 关灯

struct led_resource{
    int     gpio;   // 软件编号
    char    *name;  // 名称
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

// 设备号
static dev_t dev;
// 字符设备对象
static struct cdev led_cdev;

// 设备控制
static int led_ioctl(
        struct inode *inode,
        struct file *file,
        unsigned int cmd,
        unsigned long arg
        )
{
    // 解析cmd命令
    
    int index;
    int err;

    err = copy_from_user(&index, (int *)arg, sizeof(int));
    if(err){
        return err;
    }

    switch(cmd){
    case CMD_LED_ON:
        gpio_set_value(led_info[index-1].gpio, 1);
        break;

    case CMD_LED_OFF:
        gpio_set_value(led_info[index-1].gpio, 0);
        break;

    default:
        return -1;
    }



    // 成功返回0， 失败返回负值
    return 0;
}


// 硬件操作集合
static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .ioctl = led_ioctl,

};


static int led_init(void){
    int i;

    // 申请设备号
    alloc_chrdev_region(&dev, 0, 1, "leds");
    // 初始化cdev对象
    cdev_init(&led_cdev, &led_fops);
    // 向内核注册字符设备对象
    cdev_add(&led_cdev, dev, 1);

    // 申请GPIO资源
    for(i=0; i<ARRAY_SIZE(led_info); ++i){
        gpio_request(led_info[i].gpio, led_info[i].name);
        gpio_direction_output(led_info[i].gpio, 0);
    }

    return 0;
}

static void led_exit(void){
    int i;
    // 卸载字符设备对象
    cdev_del(&led_cdev);
    // 释放GPIO资源
    for(i=0; i<ARRAY_SIZE(led_info); ++i){
        gpio_direction_output(led_info[i].gpio, 0);
        gpio_free(led_info[i].gpio);
    }

    // 释放设备号
    unregister_chrdev_region(dev, 1);
}



module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("_Saint <CN_HuangZhibin@163.com>");
MODULE_DESCRIPTION("LED DRIVER");
MODULE_VERSION("20140827");
