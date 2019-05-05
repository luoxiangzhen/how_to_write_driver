#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/input.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>

#define DS18B20_RESET   (0x100001)
#define DS18B20_REL     (0x100002)

#define SKIP_ROM        0xcc
#define COVERTT         0x44
#define READ_MEM        0xbe
#define WRITE_MEM       0x4e

#define NINE_BIT           0x1f
#define TEN_BIT          0x3f
#define ELE_BIT          0x5f 
#define TWL_BIT          0x7f

#define TH              100
#define TL              0

static int g_rel = 12;
static struct input_dev *ds18b20_dev;
static struct delayed_work dwork;
static struct workqueue_struct *wq;

/* 参看文档P16,注意写1和0的时隙要求*/
static void ds18b20_write8(unsigned char data)
{
        int i;

        local_irq_disable();
       for (i = 0; i < 8; i++) {
                if ((data & 0x1) == 1) {
                        gpio_direction_output(S5PV210_GPH1(0), 0);
                        udelay(3);
                        gpio_direction_output(S5PV210_GPH1(0), 1);
                        udelay(80);
                } else {
                        gpio_direction_output(S5PV210_GPH1(0), 0);
                        udelay(80);
                        gpio_direction_output(S5PV210_GPH1(0), 1);
                        udelay(3);
                }
                data >>= 1;
       }
    local_irq_enable();
}

/* 参看文档P16,注意读1和0的时隙要求*/
static unsigned char ds18b20_read8(void)
{
        int i;
        unsigned char bit;
        unsigned char data = 0;

        local_irq_disable();
        for (i = 0; i < 8; i++) {
                gpio_direction_output(S5PV210_GPH1(0), 0);
                udelay(2);
                gpio_direction_input(S5PV210_GPH1(0));
                bit = gpio_get_value(S5PV210_GPH1(0));
                data |= (bit << i);
                udelay(60);
        }
    local_irq_enable();
        return data;
}

/* 参看文档P15*/
static void ds18b20_reset(void)
{
        unsigned char ret;

        gpio_direction_output(S5PV210_GPH1(0), 0);
        udelay(500);
        gpio_direction_output(S5PV210_GPH1(0), 1);
        udelay(30);
        gpio_direction_input(S5PV210_GPH1(0));
        ret = gpio_get_value(S5PV210_GPH1(0));
        udelay(500);
#if 0
        if (ret == 0) {
                printk("reset ok.\n");
        } else {
                printk("reset failed.\n");
        }
#endif
}

static void ds18b20_read(int *temp)
{
        unsigned char h8, l8;

        ds18b20_reset();
        ds18b20_write8(SKIP_ROM);
        ds18b20_write8(COVERTT);

        switch(g_rel) {
            case NINE_BIT:
                mdelay(93);
                break;
            case TEN_BIT:
                mdelay(188);
                break;
            case ELE_BIT:
                mdelay(375);
                break;
            case TWL_BIT:
                mdelay(750);
                break;
        }
        
        ds18b20_reset();
        ds18b20_write8(SKIP_ROM);
        ds18b20_write8(READ_MEM);
        
        l8 = ds18b20_read8();
        h8 = ds18b20_read8();

        *temp = (h8 << 8) | l8;

        *temp *= 625; //扩大10000倍
}

static void dwork_function(struct work_struct *work)
{
        int temperature;

        ds18b20_read(&temperature);
        input_event(ds18b20_dev, EV_MSC, MSC_RAW, temperature);
        input_sync(ds18b20_dev);
        queue_delayed_work(wq, &dwork, 2*HZ);
}

static int ds18b20_init(void)
{
        ds18b20_dev = input_allocate_device();

        ds18b20_dev->name = "DS18B20";
        set_bit(EV_MSC, ds18b20_dev->evbit);
        set_bit(MSC_RAW, ds18b20_dev->mscbit);

        input_register_device(ds18b20_dev);

        INIT_DELAYED_WORK(&dwork, dwork_function);
        gpio_request(S5PV210_GPH1(0), "GPH_8");

        wq = create_workqueue("ds18b20");
        queue_delayed_work(wq, &dwork, 0);
        return 0;
}

static void ds18b20_exit(void)
{
        cancel_delayed_work(&dwork);
        destroy_workqueue(wq);
        gpio_free(S5PV210_GPH1(0));
        input_unregister_device(ds18b20_dev);
        input_free_device(ds18b20_dev);
}

module_init(ds18b20_init);
module_exit(ds18b20_exit);
MODULE_LICENSE("GPL");
