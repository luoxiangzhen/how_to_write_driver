#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
// ioremap
#include <linux/io.h>
#include <linux/uaccess.h>

#define     LED_ON      0x100001
#define     LED_OFF     0x100002


static dev_t dev;
static struct cdev led_cdev;
static struct class *cls;
static unsigned long *gpiocon, *gpiodata;

static int led_ioctl(
        struct inode *inode, 
        struct file *file, 
        unsigned int cmd, 
        unsigned long arg)
{
    int ret;
    int index;
    ret = copy_from_user(&index, (int *)arg, sizeof(index));
    if(ret < 0){
        return -EAGAIN;
    }

    switch(cmd){
    case LED_ON:
        if(1 == index){
            // turn led1 on
            *gpiodata |= 1 << 3;
        }else if(2 == index){
            // turn led2 on
            *gpiodata |= 1 << 4;
        }
        break;

    case LED_OFF:
        if(1 == index){
            // turn led1 off
            *gpiodata &= ~(1 << 3);
        }else if(2 == index){
            // turn led2 off
            *gpiodata &= ~(1 << 4);
        }
        break;

    default:
        return -EAGAIN;
    }
    
    return 0;
}

static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .ioctl = led_ioctl,
};


static int led_init(void){
    alloc_chrdev_region(&dev, 0, 1, "leds");
    cdev_init(&led_cdev, &led_fops);
    cdev_add(&led_cdev, dev, 1);

    cls = class_create(THIS_MODULE, "LEDS");
    device_create(cls, NULL, dev, NULL, "leds");
    
    // transform phys_addr to vir_addr
    // @0xE0200080 phys_addr
    // @8 size
    // @ret vir_addr
    gpiocon = (unsigned long *)ioremap(0xE0200080, 8);
    // get led's gpio data register virtual address
    gpiodata = gpiocon + 1;
    // set gpio as output port
    *gpiocon &= ~((0xF << 12) | (0xF << 16));
    *gpiocon |= ((1 << 12) | (1 << 16));
    // turn led off
    *gpiodata &= ~((1 << 3) | (1 << 4));


    return 0;
}

static void led_exit(void){
    // turn led off
    *gpiodata &= ~((1 << 3) | (1 << 4));

    device_destroy(cls, dev);
    class_destroy(cls);
    cdev_del(&led_cdev);

    // unmap io virtual address
    iounmap(gpiocon);

    unregister_chrdev_region(dev, 1);
}   


module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
