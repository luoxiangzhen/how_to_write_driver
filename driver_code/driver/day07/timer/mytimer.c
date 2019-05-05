#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>


// 分配定时器对象
static struct timer_list timer;
static int data = 0xFFFF;

// 定时器处理函数
static void timer_func(unsigned long data){
    printk("hello, timer!\n%s data = %#X\n", 
            __func__, *(int *)data);
    mod_timer(&timer, jiffies + msecs_to_jiffies(2000));
}

static int mytimer_init(void){
    // 初始化定时器对象
    init_timer(&timer);
    timer.expires = jiffies + msecs_to_jiffies(2000);
    timer.function = timer_func;
    timer.data = (unsigned long)&data;
    // 添加注册定时器
    add_timer(&timer);

    return 0;
}

static void mytimer_exit(void){
    // 删除定时器对象
    del_timer(&timer);
}

module_init(mytimer_init);
module_exit(mytimer_exit);

MODULE_LICENSE("GPL");
