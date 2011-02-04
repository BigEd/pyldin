#include <stdio.h>
#include <stdlib.h>

static void myfunc(void)
{
    printf("In the function.\n");

   /* Go back in time: restore the execution context of setjmp
    * but make the call return 1 instead of 0. */
    exit(-1);

    printf("Not reached.\n");
}

void bye(void)
{
    printf("bye!\n");
}

int main(void)
{
    atexit(bye);
    printf("Trying some function that may throw.\n");
    myfunc();
    printf("Not reached.\n");

    return EXIT_SUCCESS;
}
