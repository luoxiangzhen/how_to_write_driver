// 1. 指定头文件
#include <linux/init.h>
#include <linux/module.h>

// 定义全局变量
static int irq;
static char* pstr;
static int fish[10];
static int nr_fish;

// 模块参数声明，加载模块或模块运行时可以修改变量内容
module_param(irq, int, 0664);
module_param(pstr, charp, 0);
module_param_array(fish, int, &nr_fish, 0664);


// 2. 指定入口函数 insmod/modprobe
static int helloworld_init(void){
    // 打印当前函数名
    printk("%s\n", __func__);
    printk("irq = %d, pstr = %s\n", irq, pstr);
    
    int idx;
    for(idx=0; idx<nr_fish; ++idx)
        printk("fish[%d] = %d\n", idx, fish[idx]);

    return 0;
}

// 3. 指定出口函数 rmmod/modprobe
static void helloworld_exit(void){
    printk("%s\n", __func__);
    printk("irq = %d, pstr = %s\n", irq, pstr);

    int idx;
    for(idx=0; idx<nr_fish; ++idx)
        printk("fish[%d] = %d\n", idx, fish[idx]);
}

// 4. 通知内核入口和出口函数
module_init(helloworld_init);
module_exit(helloworld_exit);
  
// 5. 添加模块信息 modinfo 差看
MODULE_LICENSE("GPL");  // 遵循GPL开源协议
MODULE_AUTHOR("_Saint <CN_HuangZhibin@163.com>");
MODULE_DESCRIPTION("this is my first kernel module!");
MODULE_VERSION("2014082500");
