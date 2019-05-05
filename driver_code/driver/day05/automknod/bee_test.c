#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define     BEE_ON      0x100001
#define     BEE_OFF     0x100002


int main(int argc, char *argv[]){
    if(argc != 2){
    bad_cmd:
        printf("Usage:%s <on|off>\n", argv[0]);
        return -1;
    }

    int fd = open("/dev/bee", O_RDWR);
    if(fd < 0){
        perror("open");
        return -1;
    }

    if(0 == strcmp(argv[1], "on")){
        ioctl(fd, BEE_ON);
    }else if(0 == strcmp(argv[1], "off")){
        ioctl(fd, BEE_OFF);
    }else{
        close(fd);
        goto bad_cmd;
    }

    close(fd);

    return 0;
}
