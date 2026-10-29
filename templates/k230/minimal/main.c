#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    (void)argv;
    printf("{{NAME}} on {{BOARD}} (argc=%d)\n", argc);
    for (int i = 0; i < 3; i++) { printf("tick %d\n", i); sleep(1); }
    return 0;
}
