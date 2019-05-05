#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>

struct btn_event {
    int code;
    int state;
};

int main(void)
{
    fd_set rfds; //读文件描述符集合
    int fd;
    struct btn_event btninfo; //保存按键的信息
    struct timeval tv; //监听超时时间
    int retval;
    char buf[1024] = {0};

    fd = open("/dev/mybutton", O_RDWR);
    if (fd < 0) {
        printf("open button failed.\n");
        return -1;
    }

    //启动监听
    while (1) {
        //清空读文件描述符集合
        FD_ZERO(&rfds);
        //添加监听的设备
        FD_SET(0, &rfds); //标准输入设备
        FD_SET(fd, &rfds); //开发板的按键设备
        //设置超时时间
        tv.tv_sec = 5;  //超时时间为5秒
        tv.tv_usec = 0;
    
        //正式启动监听
        //如果设备都不可用，主进程进入休眠
        printf("主进程进入休眠状态.\n");
        retval = select(fd + 1, &rfds, 
                            NULL, NULL, &tv);
        //主进程被唤醒的原因有三种
        //第一种：接收到了信号
        //第二种：超时返回
        //第三种：开发板的按键有操作或者标准输入有操作
        if (retval == -1) {
            printf("select出错!\n");
        } else if (retval == 0) {
            printf("监听超时!\n");
        } else if (retval > 0) { //有数据到来
            //判断是否是标准输入的数据引起的唤醒
            if (FD_ISSET(0, &rfds)) {
                read(0, buf, 1024);
                printf("标准输入的信息：%s\n", buf);
            }
           
            //判断是否是开发板的按键数据引起的唤醒
            if (FD_ISSET(fd, &rfds)) {
                read(fd, &btninfo, sizeof(btninfo));
                printf("开发板的按键信息：%#x, %d\n",
                        btninfo.code, btninfo.state);
            }
        }
    }
   
    close(fd);
    return 0;
}
