/*************************************************************************
	> File Name: hello_world.c
	> Author: 
	> Mail: 
	> Created Time: 2019年05月06日 星期一 16时03分01秒
 ************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("Dual BSD/GPL");


static int hello_init(void)
{
    printk(KERN_ALERT "Hello World \n");
    return 0;
}


static void hello_exit(void)
{
    printk(KERN_ALERT "Hello World exit \n");
}

module_init(hello_init);

module_exit(hello_exit);


MODULE_AUTHOR("Nolan");
MODULE_DESCRIPTION("A simple Hello World Module");
MODULE_ALIAS("a simplest module");

