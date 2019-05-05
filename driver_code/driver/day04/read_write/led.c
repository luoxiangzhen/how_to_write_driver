// 1. 引入头文件
#include <linux/init.h>
#include <linux/module.h>
// file_operations
#include <linux/fs.h>
// struct cdev
#include <linux/cdev.h>
#include <linux/device.h>
// copy_to_user/copy_from_user
// put_user/get_user
#include <linux/uaccess.h>

#include <asm/gpio.h>
#include <plat/gpio-cfg.h>

// 6. 定义LED硬件资源数据结构
struct led_resource{
    int     gpio;   // 软件编号
    char    *name;  // 名称
};

struct led_cmd{
    int value;      // 命令
    int index;      // 下标
};

// 7. 初始化LED灯的信息
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


// 8. 分配设备号和cdev对象
static dev_t dev;
static struct cdev led_cdev;

// 17. 写设备
static ssize_t led_write(
        struct file *file,
        const char __user *buf,
        size_t count,
        loff_t *ppos)
{
    // 18. 拷贝用户空间的数据到内核空间
    struct led_cmd cmd;
    int err;

    err = copy_from_user(&cmd, buf, sizeof(cmd));
    if(err){
        return err;
    }

    // 19. 解析cmd命令
    gpio_set_value(led_info[cmd.index-1].gpio, cmd.value);
    
    return count;
}

// 20. 读设备
static ssize_t led_read(
        struct file *file,
        char __user *buf,
        size_t count,
        loff_t *ppos
        )
{
    // 21. 读取灯的状态
    struct led_cmd cmd;
    int err;

    err = copy_from_user(&cmd, buf, sizeof(cmd));
    if(err){
        return err;
    }

    cmd.value = gpio_get_value(led_info[cmd.index-1].gpio);
    
    // 22. 拷贝内核空间数据到用户空间
    err = copy_to_user(buf, &cmd, sizeof(cmd));
    if(err){
        return err;
    }
    
    return count;
}



// 9. 分配初始化硬件操作集合
static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .write = led_write,     // 写设备
    .read = led_read,       // 读设备
    // open/release 可以没有
};

// 2. 入口函数
static int led_init(void){
    int i;
    // 10. 动态申请设备号
    alloc_chrdev_region(&dev, 0, 1, "leds");

    // 11. 初始化cdev对象
    //     给字符设备驱动提供操作方法
    cdev_init(&led_cdev, &led_fops);

    // 12. 向内核注册字符设备驱动
    cdev_add(&led_cdev, dev, 1);

    // 13. 申请GPIO资源,配置为输出口，输出0
    for(i=0; i<ARRAY_SIZE(led_info); ++i){
        gpio_request(led_info[i].gpio, led_info[i].name);
        gpio_direction_output(led_info[i].gpio, 0);
    }

    return 0;
}

// 3. 出口函数
static void led_exit(void){
    int i;
    // 14. 卸载cdev对象
    cdev_del(&led_cdev);

    // 15. 释放GPIO资源
    for(i=0; i<ARRAY_SIZE(led_info); ++i){
        gpio_direction_output(led_info[i].gpio, 0);
        gpio_free(led_info[i].gpio);
    }

    // 16. 释放设备号
    unregister_chrdev_region(dev, 1);
}

// 4. 注册入口/出口函数
module_init(led_init);
module_exit(led_exit);

// 5. 添加GPL信息
MODULE_LICENSE("GPL");
MODULE_AUTHOR("_Saint <CN_HuangZhibin@163.com>");
MODULE_DESCRIPTION("LED DRIVER");
MODULE_VERSION("20140827");
