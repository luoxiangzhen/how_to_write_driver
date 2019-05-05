#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define     LED_ON      0x100001
#define     LED_OFF     0x100002


int main(int argc, char *argv[]){
    if(argc != 3){
    bad_cmd:
        printf("Usage: %s <DEVICE_FILE> <on|off>\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1], O_RDWR);
    if(fd < 0){
        perror("open");
        return -1;
    }

    if(0 == strcmp(argv[2], "on")){
        ioctl(fd, LED_ON);
    }else if(0 == strcmp(argv[2], "off")){
        ioctl(fd, LED_OFF);
    }else{
        close(fd);
        goto bad_cmd;
    }
    
    close(fd);

    return 0;
}
