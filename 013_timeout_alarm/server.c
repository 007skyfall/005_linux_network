#include "net.h"

void handler(int sig)
{
    printf("alarm accured!\n");

    return ;
}

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

	//	使用alarm闹钟实现网络超时检测
	//	使用alarm设置时间，代码继续往下运行，当时间到达时，会结束整个进程
	//	如果结合信号，当闹钟响时，会触发SIGALRM信号，处理信号处理函数，当信号处理
	//函数执行完毕，会接着之前的程序继续运行，这一属性称之为自重启属性，如果
	//	想实现超时，则需要关闭这一属性
								
	//使用sigaction来修改一个信号的行为
	//注意对寄存器或者位的操作，一般执行读、改、写三步
	//第一步：获取旧的行为
	struct sigaction act;
	if(sigaction(SIGALRM, NULL, &act) < 0)
	{
		perror("fail to sigaction");
		return -1;
	}

	//第二步：修改行为
	act.sa_handler = handler;
	act.sa_flags = act.sa_flags & (~SA_RESTART);

	//第三步：将新的行为写回去
	if(sigaction(SIGALRM, &act, NULL) < 0)
	{
		perror("fail to sigaction");
		return -1;
	}

	while(1)
	{ 
		//alarm函数设置一定的时间，代码继续往下运行，当时间到达时，会结束整个进程
		alarm(5);
		
		//第五步：阻塞等待客户端的连接请求
		if((acceptfd = accept(sockfd, (struct sockaddr *)&clientaddr, &addrlen)) < 0)
		{  
			//printf("errno = %d\n", errno);
			if(errno == 4)
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
				alarm(5);
				
				if((bytes = recv(acceptfd, buf, N, 0)) < 0)
				{
					if(errno == 4)
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
					if(strncasecmp(buf, "quit", 4) == 0)
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

