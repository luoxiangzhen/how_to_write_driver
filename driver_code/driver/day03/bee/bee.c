#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include <asm/gpio.h>
#include <plat/gpio-cfg.h>

static int major;
module_param(major, int, 0);

struct bee_resource{
    int     gpio;
    char    *name;
};

static struct bee_resource bee_info[] = {
    [0] = {
        .gpio = S5PV210_GPD0(1),
        .name = "bee"
    },
};


static int bee_open(struct inode *inode, struct file *file){
    printk("%s\n", __func__);
    gpio_set_value(bee_info[0].gpio, 1);
    return 0;
}

static int bee_close(struct inode *inode, struct file *file){
    printk("%s\n", __func__);
    gpio_set_value(bee_info[0].gpio, 0);
    return 0;
}

static struct cdev bee_cdev;
static struct file_operations bee_fops = {
    .owner      = THIS_MODULE,
    .open       = bee_open,
    .release    = bee_close,
};


static int bee_init(void){
    dev_t dev;

    if(major){
        dev = MKDEV(major, 0);
        register_chrdev_region(dev, 1, "bee");
    }else{
        alloc_chrdev_region(&dev, 0, 1, "bee");
        major = MAJOR(dev);
    }
    
    cdev_init(&bee_cdev, &bee_fops);
    cdev_add(&bee_cdev, dev, 1);

    gpio_request(bee_info[0].gpio, bee_info[0].name);
    gpio_direction_output(bee_info[0].gpio, 0);
    
    return 0;
}

static void bee_exit(void){
    cdev_del(&bee_cdev);

    gpio_set_value(bee_info[0].gpio, 0);
    gpio_free(bee_info[0].gpio);

    dev_t dev = MKDEV(major, 0);
    unregister_chrdev_region(dev, 1);
}


module_init(bee_init);
module_exit(bee_exit);

MODULE_LICENSE("GPL");
