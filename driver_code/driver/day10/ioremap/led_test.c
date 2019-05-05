#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define     LED_ON       0x100001
#define     LED_OFF      0x100002

int main(int argc, char *argv[]){
    if(argc != 3){
    BAD_CMD:
        printf("Usage:<%s> <1|2> <on|off>\n", argv[0]);
        return -1;
    }

    int fd = open("/dev/leds", O_RDWR);
    if(fd < 0){
        perror("open");
        return -1;
    }

    int index = strtoul(argv[1], NULL, 0);
    
    if(strcmp(argv[2], "on") == 0){
        ioctl(fd, LED_ON, &index);
    }else if(strcmp(argv[2], "off") == 0){
        ioctl(fd, LED_OFF, &index);
    }else{
        goto BAD_CMD;
    }

    return 0;
}
