#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/irq.h>
#include <linux/interrupt.h>

static int data = 0xFFFF;

// 中断处理函数
static irqreturn_t button_isr(int irq, void *dev){
    printk("%s: irq = %d, data = %#x\n",
            __func__, irq, *(int *)dev);
    return IRQ_HANDLED;
}


static int btn_init(void){
    // 申请按键1的中断并注册按键1的中断服务程序
    request_irq(IRQ_EINT(0), 
            button_isr,
            IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
            "KEY_UP",
            &data);

    return 0;
}

static void btn_exit(void){
    // 释放中断
    // 第二个参数一定要和注册中断时传递的参数保持一致
    // 否则会造成内核崩溃
    free_irq(IRQ_EINT(0), &data);
}

module_init(btn_init);
module_exit(btn_exit);

MODULE_LICENSE("GPL");
