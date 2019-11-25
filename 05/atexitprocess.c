#include <stdlib.h>
#include <stdio.h>

void exitfunc1(void);
void exitfunc2(void);

void main() 
{
	atexit(exitfunc1);
	atexit(exitfunc2);
	printf("main function call\n");
}

void exitfunc1(void)
{
	printf("exit function 1 call.\n");
}

void exitfunc2(void)
{
	printf("exit function 2 call.\n");
}

