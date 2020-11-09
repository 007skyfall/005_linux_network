#include "net.h"

int main(int argc, const char *argv[])
{
	int sockfd;
	struct sockaddr_in serveraddr;
	socklen_t addrlen = sizeof(serveraddr);
	char buf[N] = { 0 };

	if(argc < 3)
	{
		fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
		exit(1);
	}

	//第一步：创建套接字
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		errlog("fail to socket");
	}

	//第二步：填充服务器网络信息结构体
	//inet_addr：将点分十进制ip地址转化为网络字节序的整型数据
	//htons：将主机字节序转化为网络字节序
	//atoi：将数字型字符串转化为整型数据
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	serveraddr.sin_port = htons(atoi(argv[2]));

	while(1)
	{
		if(NULL == fgets(buf, N, stdin))
		{
            fprintf(stderr, "fgets error\n");
            continue;
		}
		buf[strlen(buf) - 1] = '\0';

		if(sendto(sockfd, buf, N, 0, (struct sockaddr *)&serveraddr, addrlen) < 0)
		{
			errlog("fail to sendto");
		}

		if(strncasecmp(buf, "quit", 4) == 0)
		{
			printf("quit ...\n");
			break;
		}
		else 
		{
			if(recvfrom(sockfd, buf, N, 0, (struct sockaddr *)&serveraddr, &addrlen) < 0)
			{
				errlog("fail to recvfrom");
			}

			printf("from server : %s\n", buf);
		}
	}

	close(sockfd);
	
	return 0;
}

