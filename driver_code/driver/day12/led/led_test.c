#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

int main(int argc, char *argv[]){
    if(argc != 2){
        printf("Usage:<%s> <on|off>\n", argv[0]);
        return -1;
    }

    int fd = open("/dev/leds", O_RDWR);
    if(fd < 0){
        perror("open");
        return -1;
    }

    unsigned char *gpio_vm_base;
    unsigned long *gpiocon, *gpiodata;

    gpio_vm_base = mmap(0, 0x1000, PROT_READ | PROT_WRITE,
            MAP_SHARED, fd, 0);
    gpiocon = (unsigned long *)(gpio_vm_base + 0x80);
    gpiodata = (unsigned long *)(gpio_vm_base + 0x84);

    *gpiocon &= ~((0xF << 12) | (0xF << 16));
    *gpiocon |= ((0x1 << 12) | (0x1 << 16));
    *gpiodata &= ~((0x1 << 3) | (0x1 << 4));

    if(strcmp(argv[1], "on") == 0){
        *gpiodata |= ((1 << 3) | (1 << 4));
    }else if(strcmp(argv[1], "off") == 0){
        *gpiodata &= ~((0x1 << 3) | (0x1 << 4));
    }

    munmap(gpio_vm_base, 0x1000);
    close(fd);
    return 0;
}
