#include "net.h"

int main(int argc, const char *argv[])
{
	int sockfd;
	struct sockaddr_in serveraddr, clientaddr;
	socklen_t addrlen = sizeof(serveraddr);
	char buf[N] = { 0 };
	ssize_t bytes;

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

	//第三步：将套接字与网络信息结构体绑定
	if(bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
	{
		errlog("fail to bind");
	}

	while(1)
	{
		if((bytes = recvfrom(sockfd, buf, N, 0, (struct sockaddr *)&clientaddr, &addrlen)) < 0)
		{
			errlog("fail to recvfrom");
		}
		else
		{
			//打印获取到的客户端的信息
			printf("ip: %s, port: %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

			if(strncasecmp(buf, "quit", 4) == 0)
			{
				printf("client is quited\n");
				break;
			}
			else 
			{
				printf("from client : %s\n", buf);

				strcat(buf, " *_*");

				if(sendto(sockfd, buf, N, 0, (struct sockaddr *)&clientaddr, addrlen) < 0)
				{
					errlog("fail to sendto");
				}
			}
		}
	}	

	close(sockfd);
	
	return 0;
}

