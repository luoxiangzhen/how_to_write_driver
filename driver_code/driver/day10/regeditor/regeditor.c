#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/uaccess.h>

#define     REGEDITOR_READ      0x100001
#define     REGEDITOR_WIRTE     0x100002

struct reg_cmd{
    unsigned long addr;
    unsigned long data;
};

static dev_t dev;
static struct cdev reg_cdev;
static struct class *cls;
static unsigned long *reg_base;

static int reg_ioctl(
        struct inode *inode, 
        struct file *file, 
        unsigned int cmd, 
        unsigned long arg)
{
    struct reg_cmd reginfo;
    int ret;

    ret = copy_from_user(&reginfo, 
            (struct reg_cmd *)arg, sizeof(reginfo));
    if(ret < 0)
        return -EAGAIN;

    reg_base = ioremap(reginfo.addr, 4);
    
    switch(cmd){
    case REGEDITOR_READ:
        reginfo.data = *reg_base;
        ret = copy_to_user((struct reg_cmd *)arg,
                &reginfo, sizeof(reginfo));
        if(ret < 0){
            iounmap(reg_base);
            return -EAGAIN;
        }
        break;

    case REGEDITOR_WIRTE:
        *reg_base = reginfo.data;
        break;

    default:
        iounmap(reg_base);
        return -EAGAIN;
    }

    iounmap(reg_base);
    return 0;
}

static struct file_operations reg_fops = {
    .owner = THIS_MODULE,
    .ioctl = reg_ioctl,
};

static int regeditor_init(void){
    alloc_chrdev_region(&dev, 0, 1, "regeditor");
    cdev_init(&reg_cdev, &reg_fops);
    cdev_add(&reg_cdev, dev, 1);
    cls = class_create(THIS_MODULE, "REGEDITOR");
    device_create(cls, NULL, dev, NULL, "regeditor");

    return 0;
}

static void regeditor_exit(void){
    device_destroy(cls, dev);
    class_destroy(cls);
    cdev_del(&reg_cdev);
    unregister_chrdev_region(dev, 1);
}

module_init(regeditor_init);
module_exit(regeditor_exit);

MODULE_LICENSE("GPL");
