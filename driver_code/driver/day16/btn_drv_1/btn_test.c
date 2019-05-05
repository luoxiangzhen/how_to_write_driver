#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h> //input_event,标准键值等

int main(int argc, char *argv[])
{
    int fd;
    struct input_event button;

    fd = open(argv[1], O_RDWR); //./btn_test /dev/event1 或者是/dev/input/event1
    if (fd < 0) {
        printf("open button failed.\n");
        return -1;
    }

    while(1) {
        read(fd, &button, sizeof(button));
        printf("type = %#x, code = %#x, value = %#x\n",
                    button.type, button.code, button.value);
    }

    close(fd);
    return 0;
}
