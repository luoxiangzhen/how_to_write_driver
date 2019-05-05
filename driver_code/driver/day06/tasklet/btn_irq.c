#include <linux/init.h>
#include <linux/module.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

// 按键硬件资源
struct btn_resource{
    int     irq;    // 中断号
    char    *name;  // 中断名称
};

// 定义初始化按键信息
static struct btn_resource btn_info[] = {
    [0] = {
        .irq  = IRQ_EINT(0),
        .name = "KEY_UP"
    },
    [1] = {
        .irq  = IRQ_EINT(1),
        .name = "KEY_DOWN"
    },
    [2] = {
        .irq  = IRQ_EINT(2),
        .name = "KEY_LEFT"
    },
    [3] = {
        .irq  = IRQ_EINT(3),
        .name = "KEY_RIGHT"
    },
    [4] = {
        .irq  = IRQ_EINT(4),
        .name = "KEY_ENTER"
    },
    [5] = {
        .irq  = IRQ_EINT(5),
        .name = "KEY_BACKSPACE"
    },
    [6] = {
        .irq  = IRQ_EINT(22),
        .name = "KEY_HOME"
    },
    [7] = {
        .irq  = IRQ_EINT(23),
        .name = "KEY_POWER"
    },
};

static int data = 0xFFFF;

// tasklet 底半部
static void btn_tasklet_func(unsigned long data){
    // 底半部应该执行相对耗时的代码部分
    int i;
    for(i=0; i<10; ++i)
        mdelay(1000);
    printk("%s: data = %#X\n", __func__, *(int *)data);
}

// 分配初始化tasklet对象
static DECLARE_TASKLET(btn_tasklet, 
        btn_tasklet_func, (unsigned long)&data);

// 中断处理函数
// 顶半部
static irqreturn_t button_isr(int irq, void *dev_id){
    printk("%s: irq = %d, name = %s\n", __func__, 
            irq, ((struct btn_resource *)dev_id)->name);
    // 登记tasklet底半部
    // 登记并非去执行tasklet的延后处理函数
    tasklet_schedule(&btn_tasklet);

    return IRQ_HANDLED;
}


static int btn_irq_init(void){
    int i;
    int res;

    // 申请硬件中断资源和注册硬件中断对应的服务程序
    // @irq
    // @irq_handler_t
    // @flags
    // @name
    // @void *dev
    for(i=0; i<ARRAY_SIZE(btn_info); ++i){
        res = request_irq(
                btn_info[i].irq,
                button_isr,
                IRQF_TRIGGER_FALLING,
                btn_info[i].name,
                &btn_info[i]
        );
        if(res){
            printk("request irq error!\n");
            return res;
        }
    }

    return 0;
}


static void btn_irq_exit(void){
    int i;
    // 释放硬件中断资源和删除对应的服务程序
    // @irq
    // @void *dev_id
    for(i=0; i<ARRAY_SIZE(btn_info); ++i){
        free_irq(btn_info[i].irq, &btn_info[i]);
    }
}


module_init(btn_irq_init);
module_exit(btn_irq_exit);

MODULE_LICENSE("GPL");
