#include <linux/init.h>
#include <linux/module.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>

// 定义led硬件相关的数据结构
struct led_resource{
    int     gpio;   // GPIO软件编号
    char*   name;   // GPIO的名字
};

// 初始化led灯的资源,标记初始化
// 标记初始化无需指定顺序
// 并且可以指定某成员进行初始化
// 无需全部初始化
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

static int led_init(void){
    int i;
    for(i=0; i<ARRAY_SIZE(led_info); ++i){
        // 申请GPIO资源
        gpio_request(led_info[i].gpio, led_info[i].name);
    
        // 配置GPIO为输出口，输出1
        gpio_direction_output(led_info[i].gpio, 1);
    }

    printk("led on\n");

    return 0;
}

static void led_exit(void){
    int i;
    for(i=0; i<ARRAY_SIZE(led_info); ++i){
        // 输出0
        gpio_set_value(led_info[i].gpio, 0);
    
        // 释放GPIO资源
        gpio_free(led_info[i].gpio);
    }

    printk("led off\n");
}


module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
