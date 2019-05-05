#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>

static int major;
static struct class *cls;

static int led_ioctl(struct inode *inode,
                    struct file *file,
                    unsigned int cmd,
                    unsigned long arg)
{
    return 0;
}

struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .ioctl = led_ioctl
};

static int led_init(void)
{
    //1.注册字符设备驱动
    major = register_chrdev(major, "leds", &led_fops);
    
    //2.自动创建设备节点
    cls = class_create(THIS_MODULE, "leds");
    device_create(cls, NULL, MKDEV(major, 0), NULL, "myleds");
    
    //3.申请GPIO,配置GPIO等
    return 0;
}

static void led_exit(void)
{
    //1.释放GPIO
    //2.删除设备节点
    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);
    //3.卸载字符设备
    unregister_chrdev(major, "leds");
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
