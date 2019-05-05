#include <linux/init.h>
#include <linux/module.h>
#include "export_func.h"

void export_func(void){
    printk("%s\n", __func__);
}

// 函数的导出
EXPORT_SYMBOL(export_func);

MODULE_LICENSE("GPL");
