#include "net.h"

int main(int argc, const char *argv[])
{
	int sockfd, acceptfd;
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

	//使用select函数实现网络超时检测
	
	//注意：当select函数返回之后，会移除集合里面当前文件描述符以外其他所有的文件描述符	
	
	//注意：使用select函数设置的超时时间，设置一次，只有效一次，并且只会考虑select函数的阻塞，
	//		不会考虑对应的io操作，所以一般适用与含有一个io操作的时候使用select设置超时
	fd_set readfds, tempfds;
	int maxfd, i, ret;
	struct timeval time_out;

	//第一步：将集合清空
	FD_ZERO(&readfds);

	//第二步：将需要的文件描述符添加到集合里面
	FD_SET(sockfd, &readfds);

	maxfd = sockfd;

	while(1)
	{
		//select函数返回之后，会清空时间，所以需要每次都设置
		time_out.tv_sec = 5;
		time_out.tv_usec = 0;

		tempfds = readfds;
	
		//第三步：调用select函数阻塞等待文件描述符准备就绪
		if((ret = select(maxfd + 1, &tempfds, NULL, NULL, &time_out)) < 0)
		{
			errlog("fail to select");
		}

		//select函数设置超时时间，如果时间到达时还没有文件描述符准备就绪，则会返回0
		else if(ret == 0)
		{
			printf("timeout ...\n");
		}
		else 
		{
			//根据性质判断到底是哪个文件描述符准备就绪，并执行相应的io操作
			for(i = 0; i <= maxfd; i++)
			{
				if(FD_ISSET(i, &tempfds))
				{
					if(i == sockfd)
					{
						if((acceptfd = accept(sockfd, (struct sockaddr *)&clientaddr, &addrlen)) < 0)
						{
							errlog("fail to accept");
						}

						//打印获取到的客户端的信息
						printf("ip: %s, port: %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

						//将需要的文件描述符添加到集合里面
						FD_SET(acceptfd, &readfds);

						//判断最大的文件描述符
						maxfd = maxfd > acceptfd ? maxfd : acceptfd;
					}
					else 
					{
						if((bytes = recv(i, buf, N, 0)) < 0)
						{
							errlog("fail to recv");
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
								exit(1);
							}
							else 
							{
								printf("from client : %s\n", buf);

								strcat(buf, " *_*");

								if(send(i, buf, N, 0) < 0)
								{
									errlog("fail to send");
								}
							}
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
