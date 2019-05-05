#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h> //copy_from_user
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>

static int led_ioctl(struct inode *inode, 
                        struct file *file,
                        unsigned int cmd,
                        unsigned long arg)
{
    return 0;
}

//分配初始化硬件操作集合
static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .ioctl = led_ioctl //开关灯
};

//分配初始化混杂设备对象
static struct miscdevice led_misc = {
    .minor = MISC_DYNAMIC_MINOR, //内核分配次设备号
    .name = "myled", //内核创建设备文件/dev/myled
    .fops = &led_fops //混杂设备具有硬件的操作方法
};

static int led_init(void)
{
    //注册混杂设备对象
    misc_register(&led_misc);
    //申请GPIO，配置GPIO，输出0
    return 0;
}

static void led_exit(void)
{
    //释放GPIO资源
    //卸载混杂设备对象
    misc_deregister(&led_misc);
}
module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
