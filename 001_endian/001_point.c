#include <stdio.h>

//使用指针测试主机字节序

int main(int argc, const char *argv[])
{
	int a = 0x12345678;
	char *p;

	p = (char *)&a;

	printf("a = %#x\n", a);
	printf("*p = %#x\n", *p);

	if(*p == 0x78)
	{
		printf("little endian\n");
	}
	else 
	{
		printf("big endian\n");
	}
	
	return 0;
}

