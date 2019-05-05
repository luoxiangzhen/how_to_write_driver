#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>

#define     BEE_ON      0x100001
#define     BEE_OFF     0x100002


static dev_t dev;
static struct cdev bee_cdev;
static struct class *cls;


static int bee_ioctl(
        struct inode *inode,
        struct file *file,
        unsigned int cmd,
        unsigned long arg
        )
{
    switch(cmd){
    case BEE_ON:
        gpio_set_value(S5PV210_GPD0(1), 1);
        break;
    
    case BEE_OFF:
        gpio_set_value(S5PV210_GPD0(1), 0);
        break;
    
    default:
        return -1;
    }

    return 0;
}


static struct file_operations bee_fops = {
    .owner = THIS_MODULE,
    .ioctl = bee_ioctl,
};



static int bee_init(void){
    alloc_chrdev_region(&dev, 0, 1, "bee");
    cdev_init(&bee_cdev, &bee_fops);
    cdev_add(&bee_cdev, dev, 1);
    
    cls = class_create(THIS_MODULE, "BEE");
    device_create(cls, NULL, dev, NULL, "bee");

    gpio_request(S5PV210_GPD0(1), "bee");
    gpio_direction_output(S5PV210_GPD0(1), 0);

    return 0;
}

static void bee_exit(void){
    device_destroy(cls, dev);
    class_destroy(cls);

    cdev_del(&bee_cdev);

    gpio_set_value(S5PV210_GPD0(1), 0);
    gpio_free(S5PV210_GPD0(1));
    
    unregister_chrdev_region(dev, 1);
}



module_init(bee_init);
module_exit(bee_exit);

MODULE_LICENSE("GPL");
