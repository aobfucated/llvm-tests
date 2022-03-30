#include<stdio.h>

int main()
{
	printf("123\n");
	__builtin_return_address(0);
	return 0;
}