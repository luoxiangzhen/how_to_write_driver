#include <linux/init.h>
#include <linux/module.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

static unsigned int frequence = 200;
module_param(frequence, int, 0664);

struct led_resource{
    int     gpio;
    char    *name;
};

static struct led_resource led_info[] = {
    [0] = {
        .gpio = S5PV210_GPC1(3),
        .name = "led1"
    },
    [1] = {
        .gpio = S5PV210_GPC1(4),
        .name = "led2"
    },
};

static struct delayed_work dwork;

static void led_switch(struct work_struct *work){
    int i;

    for(i=0; i<ARRAY_SIZE(led_info); ++i)
        gpio_set_value(led_info[i].gpio,
                !gpio_get_value(led_info[i].gpio));

    schedule_delayed_work(&dwork, frequence);
}


static int led_init(void){
    int i;

    for(i=0; i<ARRAY_SIZE(led_info); ++i){
        gpio_request(led_info[i].gpio, led_info[i].name);
        gpio_direction_output(led_info[i].gpio, 0);
    }

    INIT_DELAYED_WORK(&dwork, led_switch);
    schedule_delayed_work(&dwork, frequence);

    return 0;
}

static void led_exit(void){
    int i = 0;

    for(i=0; i<ARRAY_SIZE(led_info); ++i){
        gpio_set_value(led_info[i].gpio, 0);
        gpio_free(led_info[i].gpio);
    }

    cancel_delayed_work(&dwork);
}


module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
