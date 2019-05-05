#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>


#define     CMD_LED_ON      0x100001
#define     CMD_LED_OFF     0x100002


int main(int argc, char *argv[]){
    if(argc != 3){
        printf("Usage: %s <n> <on|off> \n", argv[0]);
        return -1;
    }

    int fd = open("/dev/leds", O_RDWR);
    if(fd < 0){
        perror("open");
        return -1;
    }

    int index = strtoul(argv[1], NULL, 0);

    if(0 == strcmp(argv[2], "on")){
        ioctl(fd, CMD_LED_ON, &index);
    }else if(0 == strcmp(argv[2], "off")){
        ioctl(fd, CMD_LED_OFF, &index);
    }else{
        printf("Usage: %s <n> <on|off> \n", argv[0]);
        return -1;
    }
    
    close(fd);

    return 0;
}
