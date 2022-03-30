#include <stdio.h>

register size_t stack asm("rsp");

int main()
{
	
    printf("%zx\n", stack);
}