#include <stdio.h>

int i = 0;
float f = 3.14f;
int d = 123456;

int main(int argc, char *argv[])
{
    float f1 = 1.43f;
    int d1 = 3;

    printf("Hello world!\n");

    register volatile unsigned long val;
    asm volatile ("mov %0, sp": "=r" (val));

    printf("stack %08X\n", val);

    printf("static %f %d\n", f, d);
    printf("stack  %f %d\n", f1, d1);

    char *ptr = malloc(2048);
    if (!ptr)
	printf("malloc() return NULL!\n");
    else
	free(ptr);

    for (i = 0; i < argc; i++) {
	printf("Arg[%d] = [%s]\n", i, argv[i]);
    }
    
    printf("->%f %f\n", (float) f / i + d, (float) f1 / i + d1);

    return 0;
}
