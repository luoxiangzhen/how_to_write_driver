#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/poll.h> //poll_wait
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>
//定义板卡每一个按键信息结构体
struct btn_resource {
    int irq;
    int gpio;
    int code;
    char *name;
};

//定义板卡所有按键信息结构体
struct btn_data {
    struct btn_resource *pres;
    int nbuttons;
};

//定义上报的按键信息的结构体
struct btn_event {
    int code;   //上报的按键值
    int state;  //上报的按键状态
};

static dev_t dev;
static struct cdev btn_cdev;
static struct class *cls;
static struct btn_event g_btn; //上报的信息
static int ispressed; //=1:按键有操作;=0:按键无操作
static wait_queue_head_t btn_wq; //等待队列头，用于休眠

//[7]
static ssize_t btn_read(struct file *file,
                        char __user *buf,
                        size_t count,
                        loff_t *ppos)
{
    //7.1判断按键是否有操作，如果没有，进入休眠，如果有，上报按键信息g_btn
    wait_event_interruptible(btn_wq, ispressed);
    ispressed = 0; //清除之前，肯定为1，由中断处理函数置1

    //7.2上报按键信息g_btn,g_btn上报之前已经被中断处理函数给初始化了
    copy_to_user(buf, &g_btn, sizeof(g_btn));
    return count;
}

//[8]如果应用程序使用select来监听按键设备，必须提供poll接口
static unsigned int btn_poll(struct file *file,
                        struct poll_table_struct *wait)
{
    unsigned int mask = 0; 
    //8.1将当前进程添加到等待队列头所在的睡眠队列中
    //重点第二个参数为自己定义等待队列
    poll_wait(file, &btn_wq, wait); //不会休眠

    //8.2判断按键设备是否有操作，如果有，返回非0，如果没有，返回0，返回值给sys_select
    if (ispressed) { //有按键操作
        mask = POLLIN|POLLRDNORM; //设备可读
    }
    return mask;
}

//[5]
static struct file_operations btn_fops = {
    .owner = THIS_MODULE,
    .read = btn_read, //获取按键的信息
    .poll = btn_poll //监听按键设备，这个不是必须的
};

//[6]
//中断处理函数
//dev_id:指向每一个按键对应的硬件信息
static irqreturn_t button_isr(int irq, void *dev_id)
{
    //6.1获取每一个按键的硬件信息
    struct btn_resource *pdata = 
                (struct btn_resource *)dev_id;
    
    //6.2获取按键状态和按键值，保存在上报的变量中
    g_btn.code = pdata->code;
    g_btn.state = !gpio_get_value(pdata->gpio);

    //6.3唤醒休眠的进程、
    ispressed = 1; //记录有按键操作
    wake_up_interruptible(&btn_wq);

    return IRQ_HANDLED;
}

//[3]
//pdev指向匹配成功的硬件节点btn_dev
static int btn_probe(struct platform_device *pdev)
{
    int i;

    //3.1通过pdev获取按键硬件信息
    //pdata指向btndata(btn_dev.c里)
    struct btn_data *pdata = 
                    pdev->dev.platform_data;

    //3.2处理硬件信息（申请GPIO资源，注册中断）
    for (i = 0; i < pdata->nbuttons; i++) {
        //获取每一个按键的硬件信息
        struct btn_resource *p = &pdata->pres[i];
        gpio_request(p->gpio, p->name);
        request_irq(p->irq, 
                    button_isr,
            IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,
            p->name, p);
    }
    //3.3注册字符设备
    alloc_chrdev_region(&dev, 0, 1, "buttons");
    cdev_init(&btn_cdev, &btn_fops);
    cdev_add(&btn_cdev, dev, 1);
    cls = class_create(THIS_MODULE, "buttons");
    device_create(cls, NULL, dev, NULL, "mybutton");
    
    //3.4初始化等待队列头
    init_waitqueue_head(&btn_wq);
    return 0;
}
//[4]
//pdev指向匹配成功的硬件节点btn_dev
static int btn_remove(struct platform_device *pdev)
{
    int i;

    //4.1释放中断和GPIO资源
    struct btn_data *pdata = 
                pdev->dev.platform_data;
    
    for (i = 0; i < pdata->nbuttons; i++) {
        //获取每一个按键的硬件信息
        struct btn_resource *p = &pdata->pres[i];
        free_irq(p->irq, p);
        gpio_free(p->gpio);
    }

    //4.2卸载字符设备
    device_destroy(cls, dev);
    class_destroy(cls);
    cdev_del(&btn_cdev);
    unregister_chrdev_region(dev, 1);
    return 0;
}

//[1]
//分配初始化platform_driver
static struct platform_driver btn_drv = {
    .driver = {
        .name = "mybutton" //用于匹配
    },
    .probe = btn_probe, //匹配成功调用
    .remove = btn_remove //卸载软件或者硬件调用
};

static int btn_drv_init(void)
{
    //[2]
    //注册软件节点，进行匹配
    platform_driver_register(&btn_drv);
    return 0;
}

static void btn_drv_exit(void)
{
    //[2]
    platform_driver_unregister(&btn_drv);
}
module_init(btn_drv_init);
module_exit(btn_drv_exit);
MODULE_LICENSE("GPL");
