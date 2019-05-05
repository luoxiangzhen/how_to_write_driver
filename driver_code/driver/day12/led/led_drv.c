#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
// mmap
#include <linux/mm.h>

// 记录寄存器的物理基地址
static unsigned long gpio_phy_base;

static dev_t dev;
static struct cdev led_cdev;
static struct class *cls;


static int led_mmap(struct file *file, 
        struct vm_area_struct *vma)
{
    // 关闭cache保证数据的一致性
    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    // 地址映射
    remap_pfn_range(vma, vma->vm_start, gpio_phy_base >> 12,
            vma->vm_end - vma->vm_start, vma->vm_page_prot);
    return 0;
}

static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    // 将物理地址映射到用户空间的虚拟地址
    .mmap = led_mmap
};

// 2. 硬件和软件分配成功，内核调用led_probe
// pdev指向硬件节点led_dev
static int led_probe(struct platform_device *pdev){
    // 3.
    // 获取硬件信息
    // 处理硬件信息
    struct resource *reg_res = platform_get_resource(
            pdev, IORESOURCE_MEM, 0);
    gpio_phy_base = reg_res->start;

    // 注册字符设备 
    alloc_chrdev_region(&dev, 0, 1, "leds");
    cdev_init(&led_cdev, &led_fops);
    cdev_add(&led_cdev, dev, 1);

    cls = class_create(THIS_MODULE, "LEDS");
    device_create(cls, NULL, dev, NULL, "leds");

    return 0;    
}

static int led_remove(struct platform_device *pdev){
    device_destroy(cls, dev);
    class_destroy(cls);

    cdev_del(&led_cdev);
    unregister_chrdev_region(dev, 1);

    return 0;    
}

// 1. 分配初始化platform_driver
static struct platform_driver led_drv = {
    .driver = {
        .name = "leds"
    },
    // 匹配成功时调用led_probe
    .probe = led_probe,
    // 卸载时调用led_remove
    .remove = led_remove,
};


static int led_drv_init(void){
    // 注册软件节点，进行匹配
    platform_driver_register(&led_drv);
    return 0;
}

static void led_drv_exit(void){
    // 卸载
    platform_driver_unregister(&led_drv);
}


module_init(led_drv_init);
module_exit(led_drv_exit);

MODULE_LICENSE("GPL");
