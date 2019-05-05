#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]){
    int fd = open("/dev/bee", O_RDWR);
    if(fd < 0){
        perror("open");
        return -1;
    }

    sleep(2);

    close(fd);

    return 0;
}
