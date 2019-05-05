#include <linux/init.h> 
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>

#define ADCCON		0x00
#define ADCDLY		0x08
#define ADCDATA		0x0c
#define INTADCLR	0x18
#define ADCMUX		0x1c

#define ADC_REL		0x10001
#define ADC_PORT	0x10002

static void *reg_base; //寄存器虚拟地址

//定义等待队列头
static wait_queue_head_t adc_wq;

//数据可用标志1:可用，0  :不可用
static int EndConvert;

static ssize_t adc_read(struct file *file, char __user *buf, 
        size_t count, loff_t *ppos)
{
    unsigned long data; //转换的数据

    /*1. 开启ADC*/
    writel(readl(reg_base + ADCCON) | 1, 
            reg_base + ADCCON);

    /*2. 判断ADC数据是否可用，不可用，进行
      休眠，ADC转换结束，产生中断，进行
      唤醒
      */
    wait_event_interruptible(adc_wq, EndConvert);
    EndConvert = 0;

    /*3. 读取数据，然后上报*/
    if (readl(reg_base + ADCCON) >> 16 & 1) 
        data = readl(reg_base + ADCDATA) & 0xfff;
    else
        data = readl(reg_base + ADCDATA) & 0x3ff;

    copy_to_user(buf, &data, sizeof(data));

    return count;
}

static int adc_ioctl(struct inode *inode, struct file *file,
        unsigned int cmd, unsigned long arg)
{
    int rel;  //记录分辨率
    int port;//记录通道

    switch(cmd) {
        case ADC_REL: //设置分辨率
            copy_from_user(&rel, (int *)arg, sizeof(rel));
            if (rel == 12) 
                writel(readl(reg_base + ADCCON) | (1 << 16),
                        reg_base + ADCCON);
            else if(rel == 10) 
                writel(readl(reg_base + ADCCON) & ~(1 << 16),
                        reg_base + ADCCON);
            break;

        case ADC_PORT: //设置模拟输入通道
            copy_from_user(&port, (int *)arg, sizeof(port));
            writel(port, reg_base + ADCMUX);
            break;
        default:
            return -1;
    }
    return 0;
}

/*分配，初始化fops*/
static struct file_operations adc_fops = {
    .owner = THIS_MODULE,
    .read = adc_read,
    .ioctl = adc_ioctl,
};

/*分配，初始化miscdevice*/
static struct miscdevice adc_misc = {
    .minor = MISC_DYNAMIC_MINOR, //动态分配次设备号
    .name = "s5pv210-adc", //dev/s5pv210-adc
    .fops = &adc_fops,
};

//adc转换结束，产生中断
static irqreturn_t adc_isr(int irq, void *dev_id)
{
    /*1. 唤醒休眠的进程*/
    EndConvert = 1;
    wake_up_interruptible(&adc_wq);

    /*2. 清中断*/
    writel(0, reg_base + INTADCLR);

    return IRQ_HANDLED;
}

static int s5pv210_adc_init(void)
{
    unsigned int cur = 0;

    /*1. 注册混杂设备*/
    misc_register(&adc_misc);

    /*2. 地址映射*/
    reg_base = ioremap(0xe1700000, 0x100000);

    /*3. 硬件初始化*/
    /*3.1 设置分辨率12 设置分频系数*/
    cur = (1 << 16) | (1 << 14) | (19 << 6);
    writel(cur, reg_base + ADCCON);

    /*3.2 设置模拟输出通道*/
    cur = 1;
    writel(cur, reg_base + ADCMUX);

    /*4. 注册中断*/
    request_irq(IRQ_ADC, adc_isr, 0, "adc_irq", NULL);

    //5初始化等待队列头
    init_waitqueue_head(&adc_wq);
    return 0;
}

static void s5pv210_adc_exit(void)
{
    /*释放中断*/
    free_irq(IRQ_ADC, NULL);
    /*卸载混杂设备*/
    misc_deregister(&adc_misc);
}

module_init(s5pv210_adc_init);
module_exit(s5pv210_adc_exit);
MODULE_LICENSE("GPL");
