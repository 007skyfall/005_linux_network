#include "net.h"

#if 0

关于处理僵尸进程过程理解：
1、子进程先于父进程结束，这是产生僵尸进程的原因。
2、子进程结束后，使用SIGCHLD发信号给signal函数，signal函数接到信号后，调用系统回调函数。
3、系统回调函数使用wait () 函数的原因在于，wait可以等待子进程终止，这样就不会产生僵尸进程。

#endif

void handler(int sig)
{
    /* 阻塞等待回收子进程 */
	wait(NULL);

    return ;
}

int main(int argc, const char *argv[])
{
	int sockfd    = -1;
    int acceptfd  = -1;
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

	//使用父子进程实现tcp并发服务器	
	pid_t pid;

	//处理僵尸进程
	signal(SIGCHLD, handler);

	while(1)
	{
        
        //第五步：阻塞等待客户端的连接请求
        if((acceptfd = accept(sockfd, (struct sockaddr *)&clientaddr, &addrlen)) < 0)
        {
            errlog("fail to accept");
        }
        
        //打印获取到的客户端的信息
        printf("ip: %s, port: %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        if((pid = fork()) < 0)
		{
			errlog("fail to fork");
		}
		else if(pid > 0)  //父进程负责连接
		{
		}
		else //子进程负责通信
		{
			while(1)
			{
				if((bytes = recv(acceptfd, buf, N, 0)) < 0)
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

