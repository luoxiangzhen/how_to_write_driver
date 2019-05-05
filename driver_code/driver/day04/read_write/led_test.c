#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

struct led_cmd{
    int value;
    int index;
};

int main(int argc, char *argv[]){
    if(argc != 3){
        printf("Usage: %s <n> <on|off|stat> \n", argv[0]);
        return -1;
    }

    int fd = open("/dev/leds", O_RDWR);
    if(fd < 0){
        perror("open");
        return -1;
    }

    //  convert a string to an unsigned long integer
    //  unsigned long int strtoul(
    //      const char *nptr, 
    //      char **endptr, 
    //      int base
    //  );
    struct led_cmd cmd;
    cmd.index = strtoul(argv[1], NULL, 0);

    if(!strcmp(argv[2], "on")){
        cmd.value = 1;
    }else if(!strcmp(argv[2], "off")){
        cmd.value = 0;
    }else if(!strcmp(argv[2], "stat")){
        read(fd, &cmd, sizeof(cmd));
        printf("led%d %s\n", cmd.index, cmd.value ? "on" : "off");
        goto finish;
    }else{
        printf("Usage: %s <n> <on|off|stat> \n", argv[0]);
        return -1;
    }
    
    write(fd, &cmd, sizeof(cmd));

    close(fd);

finish:
    return 0;
}
