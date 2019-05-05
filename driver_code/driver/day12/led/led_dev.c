#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>


// 分配初始化led硬件相关信息
static struct resource led_res[] = {
    [0] = {
        .start = 0xE0200000,
        .end   = 0xE0200000 + 0x1000,
        .flags = IORESOURCE_MEM
    }
};

static void led_release(struct device *dev){}

// 分配初始化platform_device
static struct platform_device led_dev = {
    .name = "leds",
    .id = -1,
    .resource = led_res,
    .num_resources = ARRAY_SIZE(led_res),
    .dev = {
        .release = led_release
    }
};

static int led_dev_init(void){
    // 注册添加硬件节点，进行匹配
    platform_device_register(&led_dev);
    return 0;
}

static void led_dev_exit(void){
    // 卸载硬件节点
    platform_device_unregister(&led_dev);
}


module_init(led_dev_init);
module_exit(led_dev_exit);

MODULE_LICENSE("GPL");
