#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>

//定义硬件信息的结构体
struct btn_resource {
    int irq;
    int gpio;
    int code;
    char *name;
};

//定义板卡所有按键信息结构体
struct btn_data {
    struct btn_resource *pres; //指向结构体数组
    int nbuttons;           //结构体数组元素的个数
};

//初始化按键硬件信息
static struct btn_resource btn_info[] = {
    [0] = {
        .irq =  IRQ_EINT(0),
        .gpio = S5PV210_GPH0(0),
        .code = 0x50,
        .name = "KEY_UP"
    }
};

//初始板卡所有按键信息
static struct btn_data btndata = {
    .pres = btn_info, //包含按键对应的结构体数组
    .nbuttons = ARRAY_SIZE(btn_info) //数组元素的个数
};

static void btn_release(struct device *dev) {}

//分配初始platform_device节点
static struct platform_device btn_dev = {
    .name = "mybutton", //用于匹配
    .id = -1,
    .dev = {
        .platform_data = &btndata, //装载硬件信息
        //.platform_data = btn_info,错误的
        .release = btn_release
    }
};

static int btn_dev_init(void)
{
    //注册硬件节点，进行匹配
    platform_device_register(&btn_dev);
    return 0;
}

static void btn_dev_exit(void)
{
    //卸载
    platform_device_unregister(&btn_dev);
}
module_init(btn_dev_init);
module_exit(btn_dev_exit);
MODULE_LICENSE("GPL");
