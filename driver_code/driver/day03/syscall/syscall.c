#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]){
    printf("10 + 20 = %ld\n", syscall(366, 10, 20));
    return 0;
}
