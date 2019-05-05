#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define     REGEDITOR_READ       0x100001
#define     REGEDITOR_WIRTE      0x100002

struct reg_cmd{
    unsigned long addr;
    unsigned long data;
};


int main(int argc, char *argv[]){
    if(argc < 3){
    BAD_CMD:
        printf("Usage:<%s> <r|w> <addr> [data]\n", argv[0]);
        return -1;
    }

    int fd = open("/dev/regeditor", O_RDWR);
    if(fd < 0){
        perror("open");
        return -1;
    }
    
    struct reg_cmd reginfo = {
        .addr = strtoul(argv[2], NULL, 16)    
    };

    if(strcmp(argv[1], "r") == 0){
        ioctl(fd, REGEDITOR_READ, &reginfo);
        printf("addr = %#x, data = %#x 已读取\n",
                (unsigned int)reginfo.addr, 
                (unsigned int)reginfo.data);
    }else if(strcmp(argv[1], "w") == 0){
        reginfo.data = strtoul(argv[3], NULL, 16);
        ioctl(fd, REGEDITOR_WIRTE, &reginfo);
        printf("addr = %#x, data = %#x 已写入\n",
                (unsigned int)reginfo.addr, 
                (unsigned int)reginfo.data);
    }else{
        goto BAD_CMD;
    }

    return 0;
}
