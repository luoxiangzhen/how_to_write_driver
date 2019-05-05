#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]){
    int fd;
    
    // 打开设备
    fd = open("/dev/led", O_RDWR);
    if(fd < 0){
        perror("open");
        return -1;
    }

    sleep(2);

    // 关闭设备
    close(fd);
    
    return 0;
}
