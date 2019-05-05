#include <linux/init.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>
//定义按键硬件私有数据结构
struct btn_resource {
    char *name; //名称
    int irq;    //中断号
    int gpio;   //GPIO编号
    int code;   //键值
};

//初始化开发板按键信息
static struct btn_resource btn_info[] = {
    [0] = {
        .name = "KEY_L",
        .irq = IRQ_EINT(0),
        .gpio = S5PV210_GPH0(0),
        .code = KEY_L
    },
    [1] = {
        .name = "KEY_S",
        .irq = IRQ_EINT(1),
        .gpio = S5PV210_GPH0(1),
        .code = KEY_S
    },
    [2] = {
        .name = "KEY_ENTER",
        .irq = IRQ_EINT(2),
        .gpio = S5PV210_GPH0(2),
        .code = KEY_ENTER
    },
};

//定义input_dev指针
static struct input_dev *btn_dev;

//分配定时器
static struct timer_list btn_timer;
static struct btn_resource *pdata; 

//定时器的处理函数
static void btn_timer_func(unsigned long data)
{
    unsigned int pinstate;
    
    //2.获取按键的状态
    pinstate = gpio_get_value(pdata->gpio);

    //3.上报按键信息给核心层然后唤醒休眠的进程
    if (pinstate == 1) { //松开
        //EV_KEY:上报按键类事件
        //pdata->code:具体键值
        //0:松开
        input_event(btn_dev, EV_KEY, pdata->code, 0);
        input_sync(btn_dev); //上报同步类事件
    } else {    //按下
        //EV_KEY:上报按键类事件
        //pdata->code:具体键值
        //1:按下
        input_event(btn_dev, EV_KEY, pdata->code, 1);
        input_sync(btn_dev);//上报同步类事件
    }
}

//中断处理函数
static irqreturn_t button_isr(int irq, void *dev_id)
{   
    //1.获取按键对应的数据项
    pdata = (struct btn_resource *)dev_id;
    
    //2.启动定时器，设置定时器的超时时间为10ms
    mod_timer(&btn_timer, jiffies + msecs_to_jiffies(10));
    return IRQ_HANDLED;
}

static int btn_init(void)
{
    int i;

    //1.分配input_dev
    btn_dev = input_allocate_device();
    
    //2.初始化input_dev
    btn_dev->name = "tarena_button";
    //2.1设置上报按键类事件
    set_bit(EV_KEY, btn_dev->evbit);
    //2.2设置上报重复类事件
    set_bit(EV_REP, btn_dev->evbit);
    //2.3设置上报按键类事件中的哪些键值
    for(i = 0; i < ARRAY_SIZE(btn_info); i++)
            set_bit(btn_info[i].code, btn_dev->keybit);

    //3.注册input_dev
    input_register_device(btn_dev);
    
    //4.申请GPIO资源
    //5.注册中断处理函数
    for (i = 0; i < ARRAY_SIZE(btn_info); i++) {
        gpio_request(btn_info[i].gpio, btn_info[i].name);
        request_irq(btn_info[i].irq, button_isr,
                IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,
                btn_info[i].name, &btn_info[i]);
    }
    
    //6.初始化定时器
    init_timer(&btn_timer);
    
    //6.1指定定时器的处理函数
    btn_timer.function = btn_timer_func;
    
    return 0;
}

static void btn_exit(void)
{
    int i;

    //1.释放中断，释放GPIO资源
    for (i = 0; i < ARRAY_SIZE(btn_info); i++) {
        gpio_free(btn_info[i].gpio);
        free_irq(btn_info[i].irq, &btn_info[i]);
    }

    //2.卸载input_dev
    input_unregister_device(btn_dev);

    //3.释放input_dev内存
    input_free_device(btn_dev);
    
    //4.删除定时器
    del_timer(&btn_timer);
}

module_init(btn_init);
module_exit(btn_exit);
MODULE_LICENSE("GPL");
