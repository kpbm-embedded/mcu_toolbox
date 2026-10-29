#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    printf("hello from K230 RT-Smart user space (argc=%d)\n", argc);
    for (int i = 0; i < 3; i++) {
        printf("tick %d\n", i);
        sleep(1);
    }
    return 0;
}
