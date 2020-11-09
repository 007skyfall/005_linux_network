#include <stdio.h>

//使用共用体测试主机字节序

union un{
	int a;
	char b;
};

int main(int argc, const char *argv[])
{
	union un myun;

	myun.a = 0x12345678;

	printf("a = %#x\n", myun.a);
	printf("b = %#x\n", myun.b);

	if(myun.b == 0x78)
	{
		printf("littile endian\n");
	}
	else 
	{
		printf("big endian\n");
	}
	
	return 0;
}
