#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/input.h>

int main(int argc, char *argv[])
{
        int fd;
        struct input_event data;

        fd = open(argv[1], O_RDWR);
        if (fd < 0) {
                printf("open data failed.\n");
                exit(-1);
        }

        while (1) {
                read(fd, &data, sizeof(data));
                if (data.type == EV_MSC)
                    printf("%.3f\n", (float)data.value/ 10000);        
        }
        close(fd);
        return 0;
}
