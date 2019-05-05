#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>

static dev_t dev;
static struct cdev chrdev_cdev;

static int chrdev_open(
        struct inode *inode,
        struct file *file
        );

static int chrdev_close(
        struct inode *inode,
        struct file *file
        );

static ssize_t chrdev_read(
        struct file *file,
        char __user *buf,
        size_t count,
        loff_t *ppos
        );

static ssize_t chrdev_wirte(
        struct file *file,
        char const __user *buf,
        size_t count,
        loff_t *ppos
        );

static int chrdev_ioctl(
        struct inode *inode,
        struct file *file,
        unsigned int cmd,
        unsigned long arg
        );

static struct file_operations chrdev_fops = {
    .owner = THIS_MODULE,
    .open = chrdev_open,
    .release = chrdev_close,
    .read = chrdev_read,
    .write = chrdev_write,
    .ioctl = chrdev_ioctl,
}



static int chrdev_init(void){
    alloc_chrdev_region(&dev, 0, 1, "chrdev");
    cdev_init(&chrdev_cdev, &chrdev_fops);
    cdev_add(&chrdev_cdev, dev, 1);
    
    gpio_request(S5PV210_GPIO(n), "S5PV210_GPIO(n)");
    gpio_direction_output(S5PV210_GPIO(n), init_value);

    return 0;
}

static void chrdev_exit(void){
    gpio_set_value(S5PV210_GPIO(n), close_value);
    gpio_free(S5PV210_GPIO(n));

    cdev_del(&chrdev);
    unregister_chrdev_region(dev, 1);
}



module_init(chrdev_init);
module_exit(chrdev_exit);

MODULE_LICENSE("GPL");
