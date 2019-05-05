// [1]
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>


// [3] 定义按键硬件数据结构
struct btn_resource{
    int     irq;    // 中断号
    int     gpio;   // GPIO编号
    char    *name;  // 名称
    int     code;   // 按键值
};

// [5] 定义上报按键的信息数据结构
struct btn_event{
    int     code;   // 上报的按键值
    int     state;  // 上报的按键状态
                    // 1-按下，0-松开
};

// [4] 分配初始化按键信息
static struct btn_resource btn_info[] = {
    [0] = {
        .irq  = IRQ_EINT(0),
        .gpio = S5PV210_GPH0(0),
        .name = "KEY_UP",
        .code = 0x50
    },
    [1] = {
        .irq  = IRQ_EINT(1),
        .gpio = S5PV210_GPH0(1),
        .name = "KEY_DOWN",
        .code = 0x51
    },
};

// [6] 定义按键操作信息相关对象
static struct btn_event g_btn_data;  // 记录操作的按键信息
static int ispressed;                // 记录按键是否有操作
static wait_queue_head_t btn_wqh;    // 分配等待队列头

// [7]
static dev_t dev;
static struct cdev btn_cdev;
static struct class *cls;

// [10]
static ssize_t btn_read(struct file *file,
        char __user *buf, size_t count, loff_t *ppos)
{
    int res;

    // 给每一个进程都分配一个存放进程的容器对象
    // @wait 存放当前进程的容器
    // @current 当前进程
    DECLARE_WAITQUEUE(wait, current);

    printk("进程[%s]%d将进入休眠状态\n",
            current->comm, current->pid);
    // 将当前进程添加到等待队列中
    add_wait_queue(&btn_wqh, &wait);
    // 设置当前进程的休眠状态为可中断状态
    set_current_state(TASK_INTERRUPTIBLE);
    // 进入真正的休眠状态
    schedule();
    // 一旦进程被信号或者中断唤醒，schedule函数返回
    set_current_state(TASK_RUNNING);
    remove_wait_queue(&btn_wqh, &wait);
    // 判断是否接收到信号
    if(signal_pending(current)){
        printk("进程[%s]%d被信号唤醒\n",
                current->comm, current->pid);
        return -ERESTARTSYS; // 重启应用软件
    }
    // 为下一次获取按键做准备
    ispressed = 0;
    // 上报按键信息
    res = copy_to_user(buf, &g_btn_data, sizeof(g_btn_data));
    printk("进程[%s]%d被中断唤醒\n",
            current->comm, current->pid);

    return count;
}

// [11]
static irqreturn_t btn_isr(int irq, void *dev_id){
    // 通过dev_id获得按键对应的硬件信息
    struct btn_resource *pdata = (struct btn_resource *)dev_id;
    // 获取按键的状态
    // 按下上报1，松开上报0，与硬件电平相反
    g_btn_data.state = !gpio_get_value(pdata->gpio);
    g_btn_data.code = pdata->code;

    // 唤醒休眠的进程
    ispressed = 1; // 记录按键有操作
    wake_up_interruptible(&btn_wqh);

    return IRQ_HANDLED;
}

// [7]
static struct file_operations btn_fops = {
    .owner = THIS_MODULE,
    .read = btn_read,   // 获取按键信息
};


static int btn_init(void){
    int i;
    int res;

    // [8]
    // 申请设备号
    alloc_chrdev_region(&dev, 0, 1, "btns");
    // 初始化字符设备对象
    cdev_init(&btn_cdev, &btn_fops);
    // 注册字符设备对象
    cdev_add(&btn_cdev, dev, 1);
    // 自动创建设备文件
    cls = class_create(THIS_MODULE, "KEYS");
    device_create(cls, NULL, dev, NULL, "btns");

    // 申请gpio资源，申请中断号资源，注册中断处理函数
    for(i=0; i<ARRAY_SIZE(btn_info); ++i){
        gpio_request(btn_info[i].gpio, btn_info[i].name);
        res = request_irq(
                btn_info[i].irq,
                btn_isr,
                IRQF_TRIGGER_FALLING | 
                IRQF_TRIGGER_RISING,
                btn_info[i].name,
                &btn_info[i]
        );
    }
    
    // 初始化等待队列头对象
    init_waitqueue_head(&btn_wqh);

    return 0;
}

static void btn_exit(void){
    int i;
    // [9]
    device_destroy(cls, dev);
    class_destroy(cls);

    for(i=0; i<ARRAY_SIZE(btn_info); ++i){
        free_irq(btn_info[i].irq, &btn_info[i]);
        gpio_free(btn_info[i].gpio);
    }

    cdev_del(&btn_cdev);
    unregister_chrdev_region(dev, 1);
}


// [2]
module_init(btn_init);
module_exit(btn_exit);

MODULE_LICENSE("GPL");
