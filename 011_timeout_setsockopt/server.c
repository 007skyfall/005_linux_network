#include "net.h"

int main(int argc, const char *argv[])
{
	int sockfd, acceptfd;
	struct sockaddr_in serveraddr, clientaddr;
	socklen_t addrlen = sizeof(serveraddr);
	char buf[N] = { 0 };
	ssize_t  bytes;

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

	//第三步：将套接字与网络信息结构体绑定
	if(bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
	{
		errlog("fail to bind");
	}

	//第四步：将套接字设置为被动监听状态
	if(listen(sockfd, 5) < 0)
	{
		errlog("fail to listen");
	}

	//使用setsockopt实现网络超时检测
	struct timeval rcv_time;
	rcv_time.tv_sec  = 5;
	rcv_time.tv_usec = 0;

	if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &rcv_time, sizeof(rcv_time)) < 0)
	{
		errlog("fail to setsockopt");
	}

	while(1)
	{
		//第五步：阻塞等待客户端的连接请求
		if((acceptfd = accept(sockfd, (struct sockaddr *)&clientaddr, &addrlen)) < 0)
		{ 
			//printf("errno = %d\n", errno);
			if(errno == 11)
			{
				printf("accept timeout ...\n");
			}
			else 
			{
				errlog("fail to accept");
			}
		}
		else 
		{
			//打印获取到的客户端的信息
			printf("ip: %s, port: %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

			while(1)
			{
				if((bytes = recv(acceptfd, buf, N, 0)) < 0)
				{
					if(errno == 11)
					{
						strcpy(buf, "TIMEO");
						send(acceptfd, buf, N, 0);

						printf("The client is timeout, must be quited...\n");
						close(acceptfd);

						break;
					}
					else 
					{
						errlog("fail to recv");
					}
				}
				else if(bytes == 0)
				{
					printf("NO DATA\n");
					exit(1);
				}
				else
				{
					if(strncmp(buf, "quit", 4) == 0)
					{
						printf("client is quited\n");
						break;
					}
					else 
					{
						printf("from client : %s\n", buf);

						strcat(buf, " *_*");

						if(send(acceptfd, buf, N, 0) < 0)
						{
							errlog("fail to send");
						}
					}
				}
			}	
		}
	}

	close(acceptfd);
	close(sockfd);
	
	return 0;
}

