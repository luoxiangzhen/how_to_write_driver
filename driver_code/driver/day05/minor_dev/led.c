#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>

#define     LED_ON      0x100001
#define     LED_OFF     0x100002

struct led_resource{
    int     gpio;
    char    *name;
};

static struct led_resource led_info[] = {
    [0] = {
        .gpio = S5PV210_GPC1(3),
        .name = "led1"
    },
    [1] = {
        .gpio = S5PV210_GPC1(4),
        .name = "led2"
    }
};


static dev_t dev;
static struct cdev led_cdev;
static struct class *cls;


static int led_ioctl(
        struct inode *inode,
        struct file *file,
        unsigned int cmd,
        unsigned long arg
        )
{
    int minor = MINOR(inode->i_rdev);

    // file与inode同样存在关系
    // int minor = MINOR(file->f_path.dentry->d_inode->i_rdev);
    switch(cmd){
    case LED_ON:
        gpio_set_value(led_info[minor].gpio, 1);
        break;

    case LED_OFF:
        gpio_set_value(led_info[minor].gpio, 0);
        break;

    default:
        return -1;
    }

    return 0;
}


static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .ioctl = led_ioctl,
};


static int led_init(void){
    int i;

    alloc_chrdev_region(&dev, 0, 2, "leds");
    cdev_init(&led_cdev, &led_fops);
    cdev_add(&led_cdev, dev, 2);

    // 自动创建设备节点
    cls = class_create(THIS_MODULE, "leds");
    device_create(cls, NULL, MKDEV(MAJOR(dev), 0), NULL, "led1");
    device_create(cls, NULL, MKDEV(MAJOR(dev), 1), NULL, "led2");

    for(i=0; i<ARRAY_SIZE(led_info); ++i){
        gpio_request(led_info[i].gpio, led_info[i].name);
        gpio_direction_output(led_info[i].gpio, 0);
    }
    
    return 0;
}

static void led_exit(void){
    int i;

    // 删除设备节点
    device_destroy(cls, MKDEV(MAJOR(dev), 0));
    device_destroy(cls, MKDEV(MAJOR(dev), 1));
    class_destroy(cls);

    for(i=0; i<ARRAY_SIZE(led_info); ++i){
        gpio_set_value(led_info[i].gpio, 0);
        gpio_free(led_info[i].gpio);
    }

    cdev_del(&led_cdev);
    unregister_chrdev_region(dev, 2);
}


module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
