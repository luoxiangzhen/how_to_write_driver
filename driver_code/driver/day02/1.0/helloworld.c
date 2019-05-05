// 1. 指定头文件
#include <linux/init.h>
#include <linux/module.h>

// 2. 指定入口函数 insmod/modprobe
static int helloworld_init(void){
    // 打印当前函数名
    printk("%s\n", __func__);
    return 0;
}

// 3. 指定出口函数 rmmod/modprobe
static void helloworld_exit(void){
    printk("%s\n", __func__);
}

// 4. 通知内核入口和出口函数
module_init(helloworld_init);
module_exit(helloworld_exit);
  
// 5. 添加模块信息 modinfo 差看
MODULE_LICENSE("GPL");  // 遵循GPL开源协议
MODULE_AUTHOR("_Saint <CN_HuangZhibin@163.com>");
MODULE_DESCRIPTION("this is my first kernel module!");
MODULE_VERSION("2014082500");
