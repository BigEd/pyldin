/*
 * Example from http://blog.julipedia.org/2011/01/understanding-setjmplongjmp.html
 */

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

static jmp_buf buf;

static void myfunc(void)
{
   printf("In the function.\n");

   /* Go back in time: restore the execution context of setjmp
    * but make the call return 1 instead of 0. */
   longjmp(buf, 1);

   printf("Not reached.\n");
}

int main(void)
{
   if (setjmp(buf) == 0) {
       /* Try block. */
       printf("Trying some function that may throw.\n");
       myfunc();
       printf("Not reached.\n");
   } else {
       /* Catch block. */
       printf("Exception caught.\n");
   }
   return EXIT_SUCCESS;
}
