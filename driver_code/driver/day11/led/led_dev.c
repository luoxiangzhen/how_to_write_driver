#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

// 2.
struct led_resource{
    char    *name;
    int     productid;
};

// 3. 初始化硬件信息
static struct led_resource led_info = {
    .name = "tarena",
    .productid = 0x123456
};

// 4. 分配初始化resource类型的硬件资源信息
static struct resource led_res[] = {
    [0] = {
        .start = 0xE0200080,
        .end = 0xE0200080 + 8,
        .flags = IORESOURCE_MEM
    },
    [1] = {
        .start = 3,     // GPIO管脚的硬件编号
        .end = 3,
        .flags = IORESOURCE_IRQ // IO资源
    }
};

static void led_release(struct device *dev){}

// 1.
static struct platform_device led_dev = {
    .name = "leds",
    .id = -1,
    .dev = {
        .platform_data = &led_info,
        .release = led_release,
    },
    .resource = led_res,
    .num_resources = ARRAY_SIZE(led_res),
};

static int led_dev_init(void){
    platform_device_register(&led_dev);
    return 0;
}

static void led_dev_exit(void){
    platform_device_unregister(&led_dev);
}


module_init(led_dev_init);
module_exit(led_dev_exit);

MODULE_LICENSE("GPL");
