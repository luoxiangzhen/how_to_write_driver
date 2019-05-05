#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#define     LED_ON      0x100001
#define     LED_OFF     0x100002

struct led_resource{
    char    *name;
    int     productid;
};

static dev_t dev;
static struct cdev led_cdev;
static struct class *cls;
static int pin;
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
            // turn led1 on
            *gpiodata |= 1 << pin;
        break;

    case LED_OFF:
            // turn led1 off
            *gpiodata &= ~(1 << pin);
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

static int led_probe(struct platform_device *pdev){
    struct resource *reg_res;
    struct resource *pin_res;
    // 获取硬件信息
    struct led_resource *pdata = pdev->dev.platform_data;
    printk("led_info: name = %s, id = %#x", 
            pdata->name, pdata->productid);
    
    // @pdev - 硬件节点的首地址
    // @IORESOURCE_MEM - 资源的类型
    // @0 - 同类资源的偏移量
    // 获取寄存器资源
    reg_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    // 获取GPIO管脚资源
    pin_res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
    
    // 处理硬件信息
    // 地址映射
    gpiocon = ioremap(reg_res->start, 
            reg_res->end - reg_res->start);
    gpiodata = gpiocon + 1;
    pin = pin_res->start;
    
    // 配置为输出口，输出0
    *gpiocon &= ~(0xF << (pin*4));
    *gpiocon |= (0x1 << (pin*4));
    *gpiodata &= ~(0x1 << pin);

    // 注册字符设备
    alloc_chrdev_region(&dev, 0, 1, "leds");
    cdev_init(&led_cdev, &led_fops);
    cdev_add(&led_cdev, dev, 1);
    cls = class_create(THIS_MODULE, "LEDS");
    device_create(cls, NULL, dev, NULL, "leds");

    return 0; // 成功返回0，失败返回-1
}


static int led_remove(struct platform_device *pdev){
    *gpiodata &= ~(0x1 << pin);

    device_destroy(cls, dev);
    class_destroy(cls);
    cdev_del(&led_cdev);

    // unmap io virtual address
    iounmap(gpiocon);

    unregister_chrdev_region(dev, 1);

    return 0; // 成功返回0，失败返回-1
}


// 1. 分配初始化软件节点对象
static struct platform_driver led_drv = {
    .driver = {
        .name = "leds"
    },
    .probe = led_probe,
    .remove = led_remove
};

static int led_drv_init(void){
    // 2. 注册
    platform_driver_register(&led_drv);
    return 0;
}

static void led_drv_exit(void){
    // 3. 卸载
    platform_driver_unregister(&led_drv);
}


module_init(led_drv_init);
module_exit(led_drv_exit);

MODULE_LICENSE("GPL");
