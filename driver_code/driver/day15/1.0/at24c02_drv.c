#include <linux/module.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>

#define DELAY(us)           time_delay_us(us)

#define SCL                 (1 << 0)   
#define SDA                 (1 << 1)   

#define GPIO_I2C_READ 0x100001
#define GPIO_I2C_WRITE 0x100002

struct eeprom_data {
    unsigned char addr;
    unsigned char data;
};

static void i2c_clr(unsigned char whichline)
{
	if(whichline == SCL)
	{
	    gpio_direction_output(S5PV210_GPD1(1), 0);	
            return;
	}
	else if(whichline == SDA)
	{
            gpio_direction_output(S5PV210_GPD1(0), 0);	
            return;
	}
	else if(whichline == (SDA|SCL))
	{
            gpio_direction_output(S5PV210_GPD1(1), 0);	
            gpio_direction_output(S5PV210_GPD1(0), 0);	
            return;
	}
	else
	{
		printk("Error input.\n");
		return;
	}
	
}

static void  i2c_set(unsigned char whichline)
{
	if(whichline == SCL)
	{
            gpio_direction_output(S5PV210_GPD1(1), 1);	
            return;
	}
	else if(whichline == SDA)
	{
            gpio_direction_output(S5PV210_GPD1(0), 1);	
            return;
	}
	else if(whichline == (SDA|SCL))
	{
            gpio_direction_output(S5PV210_GPD1(1), 1);	
            gpio_direction_output(S5PV210_GPD1(0), 1);	
            return;
	}
	else
	{
		printk("Error input.\n");
		return;
	}
}

void time_delay_us(unsigned int usec)
{
    udelay(2);
}

 
static unsigned char i2c_data_read(void)
{
	unsigned char regvalue;
	
	gpio_direction_input(S5PV210_GPD1(0));
        DELAY(1);
		
        regvalue = gpio_get_value(S5PV210_GPD1(0));
        return regvalue;
}

/*
	SCL: ---------- 		1
		    
		     			0
	SDA: -----			1
		 |
		 -----			0
		START
*/
static void i2c_start_bit(void)
{
        DELAY(1);
        i2c_set(SDA | SCL);
        DELAY(1);
        i2c_clr(SDA);
        DELAY(1);
}

/*
	SCL: ----- 	       -------------	1		
		 |	       |    
		 ---------------   		0
	SDA: ---------		   ---------	1
	       ACK   |	           |
		     ---------------		0
				STOP
*/			
static void i2c_stop_bit(void)
{
        /* ACK 时钟*/
        DELAY(1);
        i2c_set(SCL);
        DELAY(1); 
        i2c_clr(SCL);  
        /* stop bit */
        DELAY(1);
        i2c_clr(SDA);
        DELAY(1);
        i2c_set(SCL);
        DELAY(1);
        i2c_set(SDA);
        DELAY(1);
}
/*
	SCL:  	    -----		1
		    |	|    
	    ---------	-----  		0
	SDA:    -------------		1   
	         发送 1  	         
					0  
	SCL:  	    -----		1
		    |	|    
	    ---------	-----  		0
	SDA:    			1

		-------------		0
		发送  0	   
*/			

static void i2c_send_byte(unsigned char c)
{
    int i;
    local_irq_disable(); //关闭中断
    for (i=0; i<8; i++)
    {
        DELAY(1);
        i2c_clr(SCL);
        DELAY(1);

        if (c & (1<<(7-i)))
            i2c_set(SDA);
        else
            i2c_clr(SDA);

        DELAY(1);
        i2c_set(SCL);
        DELAY(1);
        i2c_clr(SCL);
    }
    DELAY(1);
    local_irq_enable(); //时能中断
}

/*
	SCL:  	 ----------   		 1
	    	 |        |
	     -----	  -----		 0
	SDA:          -----		 1  
	             获取数据	         
					 0  
*/			
static unsigned char i2c_receive_byte(void)
{
    int j=0;
    int i;
    
    local_irq_disable(); //关中断
    for (i=0; i<8; i++)
    {
        DELAY(1);
        i2c_clr(SCL);
        DELAY(1);
        i2c_set(SCL);
        
        gpio_direction_input(S5PV210_GPD1(0));
        DELAY(1);
        
        if (i2c_data_read())
            j+=(1<<(7-i));

        DELAY(1);
        i2c_clr(SCL);
    }
    local_irq_enable(); //使能中断
    DELAY(1);

    return j;
}

/*
	SCL:  	 ----------   		1
	    	 |        |
	     -----	  -----		0
	SDA:          			1

		      -----		0   
	             获取ACK	           
*/			
static int i2c_receive_ack(void)
{
    int nack;
    
    DELAY(1);
    
    gpio_direction_input(S5PV210_GPD1(0)); 
    DELAY(1);
    i2c_clr(SCL);
    DELAY(1);
    i2c_set(SCL);
    DELAY(1);
    nack = i2c_data_read();
    DELAY(1);
    i2c_clr(SCL);
    DELAY(1);
    if (nack == 0)
        return 1; 

    return 0;
}

static unsigned char gpio_i2c_read(
            unsigned char devaddress, 
            unsigned char address)
{
    int rxdata;
    
    i2c_start_bit();
    i2c_send_byte(devaddress << 1);
    i2c_receive_ack();
    i2c_send_byte(address);
    i2c_receive_ack();   
    i2c_start_bit();
    i2c_send_byte(devaddress << 1 | 1);
    i2c_receive_ack();
    rxdata = i2c_receive_byte();
    i2c_stop_bit();

    return rxdata;
}


static void gpio_i2c_write(
            unsigned char devaddress, 
            unsigned char address, 
            unsigned char data)
{
    i2c_start_bit();
    i2c_send_byte(devaddress << 1);
    i2c_receive_ack();
    i2c_send_byte(address);
    i2c_receive_ack();
    i2c_send_byte(data); 
    i2c_stop_bit();
}


static int gpioi2c_ioctl(struct inode *inode, 
                            struct file *file, 
                            unsigned int cmd, 
                            unsigned long arg)
{

        struct eeprom_data eeprom;

        //1.拷贝用户访问的数据信息到内核空间
        //必须包含用户访问的EEPROM内部某个地址
        copy_from_user(&eeprom, 
                    (struct eeprom_data *)arg, 
                        sizeof(eeprom));
	
        //2.解析命令
        switch(cmd)
	{
		case GPIO_I2C_READ: //读
                        //第一个参数：设备地址
                        //第二个参数：要读的地址
                        //返回值：地址里的数据
			eeprom.data = 
                            gpio_i2c_read(0x50, 
                                    eeprom.addr);
                        //再把读到的数据拷贝到用户空间
                        copy_to_user(
                        (struct eeprom_data*)arg, 
                        &eeprom, 
                        sizeof(eeprom));
                        break;
		
		case GPIO_I2C_WRITE: //写
                        //第一个参数：设备地址
                        //第二个参数：访问的内部地址
                        //第三个参数：写入的数据
			gpio_i2c_write(0x50, 
                                eeprom.addr, 
                                eeprom.data);
                        break;		
	
		default:
			return -1;
	}
    return 0;
}

static struct file_operations gpioi2c_fops = {
    .owner      = THIS_MODULE,
    .ioctl      = gpioi2c_ioctl, //读写功能
};


static struct miscdevice gpioi2c_dev = {
   .minor		= MISC_DYNAMIC_MINOR, //动态分配次设备号
   .name		= "gpioi2c", //dev/gpioi2c
   .fops  = &gpioi2c_fops, //操作硬件的方法
};

static int gpio_i2c_init(void)
{
    int ret;

    //1.注册混杂设备
    ret = misc_register(&gpioi2c_dev);
    if(0 != ret)
    	return -1;
    
    //2.申请SCL和SDA对应的GPIO
    gpio_request(S5PV210_GPD1(0), "SDA0");
    gpio_request(S5PV210_GPD1(1), "SCL0");
  
    //3.配置SCL和SDA的管脚为输出口，并且输出高电平
    i2c_set(SCL | SDA);
    
    return 0;    
}

static void gpio_i2c_exit(void)
{
    //1.卸载混杂设备
    misc_deregister(&gpioi2c_dev);
    
    //2.释放GPIO资源
    gpio_free(S5PV210_GPD1(0));
    gpio_free(S5PV210_GPD1(1));
}


module_init(gpio_i2c_init);
module_exit(gpio_i2c_exit);

MODULE_LICENSE("GPL");

