/* 内核编程使用的头文件一律是内核源码中的头文件 */
/* 在编译驱动时，要利用内核源码/opt/kernel/include */
#include <linux/init.h>
#include <linux/module.h>

/* 编写驱动的入口函数 */
static int helloworld_init(void){
    // 入口函数一般会做向内核注册功能函数 
    // 和申请资源的工作 
    // 比如向内核注册操作硬件的函数或者申请内存资源 
    printk("hello, world!\n");
    return 0; // 如果成功，返回0，失败返回负值
}

/* 编写驱动的出口函数 */
static void helloworld_exit(void){
    // 出口函数一般会做跟入口函数相反的内容
    // 比如卸载操作硬件的函数或者释放申请的内存资源
    printk("goodbye!\n");
}

/* 告诉内核驱动的入口函数和出口函数 */
module_init(helloworld_init);
module_exit(helloworld_exit);
