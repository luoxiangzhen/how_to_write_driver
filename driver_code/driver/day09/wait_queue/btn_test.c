#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

struct key_event{
    int     code;
    int     state;
};

int main(int argc, char *argv[]){
    int fd = open("/dev/btns", O_RDWR);
    if(fd < 0){
        perror("open");
        return -1;
    }

    struct key_event key_data;

    while(1){
        if(read(fd, &key_data, sizeof(key_data)) < 0){
            perror("read");
            return -1;
        }
        printf("code = %#X, state = %d\n",
                key_data.code, key_data.state);        
    }

    close(fd);

    return 0;
}
