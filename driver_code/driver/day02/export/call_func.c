#include <linux/init.h>
#include <linux/module.h>
#include "export_func.h"

static int call_func_init(void){
    printk("%s\n", __func__);
    export_func();
    return 0;
}

static void call_func_exit(void){
    printk("%s\n", __func__);
    export_func();
}

module_init(call_func_init);
module_exit(call_func_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("_Saint <CN_HuangZhibin@163.com>");
MODULE_DESCRIPTION("multfiles");
MODULE_VERSION("2014082500");
