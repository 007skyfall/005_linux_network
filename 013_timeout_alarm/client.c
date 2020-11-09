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
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
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

#if 0
	//客户端也可以自己填充自己的信息
	struct sockaddr_in clientaddr;
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_addr.s_addr = inet_addr(argv[3]);
	clientaddr.sin_port = htons(atoi(argv[4]));

	if(bind(sockfd, (struct sockaddr *)&clientaddr, addrlen) < 0)
	{
		errlog("fail to bind");
	}
#endif

	//第三步：发送客户端的连接请求
	if(connect(sockfd, (struct sockaddr *)&serveraddr, addrlen) < 0)
	{
		errlog("fail to connect");
	}
	
	while(1)
	{
		if(!fgets(buf, N, stdin))
		{
            buf[strlen(buf) - 1] = '\0';
		}
		if(send(sockfd, buf, N, 0) < 0)
		{
			errlog("fail to send");
		}

		if(strncmp(buf, "quit", 4) == 0)
		{
			printf("quit ...\n");
			break;
		}
		else 
		{
			if(recv(sockfd, buf, N, 0) < 0)
			{
				errlog("fail to recv");
			}

			if(strncasecmp(buf, "TIMEO", 5) == 0)
			{
				printf("timeout..., quit...!!!\n");
				break;
			}

			printf("from server : %s\n", buf);
		}
	}
	
	close(sockfd);
	
	return 0;
}

