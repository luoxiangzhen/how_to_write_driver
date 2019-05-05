#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>

static unsigned int frequence = 1000;
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

static struct timer_list timer;

static void led_switch(unsigned long data){
    int i;

    for(i=0; i<ARRAY_SIZE(led_info); ++i)
        gpio_set_value(led_info[i].gpio,
                !gpio_get_value(led_info[i].gpio));
    mod_timer(&timer, jiffies + msecs_to_jiffies(frequence));
}


static int led_init(void){
    int i;

    for(i=0; i<ARRAY_SIZE(led_info); ++i){
        gpio_request(led_info[i].gpio, led_info[i].name);
        gpio_direction_output(led_info[i].gpio, 0);
    }

    init_timer(&timer);
    timer.expires = jiffies + msecs_to_jiffies(frequence);
    timer.function = led_switch;
    timer.data = 0;
    
    add_timer(&timer);

    return 0;
}

static void led_exit(void){
    int i = 0;

    for(i=0; i<ARRAY_SIZE(led_info); ++i){
        gpio_set_value(led_info[i].gpio, 0);
        gpio_free(led_info[i].gpio);
    }

    del_timer(&timer);
}


module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
