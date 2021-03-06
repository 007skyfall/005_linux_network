#include "net.h"

int main(int argc, const char *argv[])
{
	int sockfd, acceptfd;
	struct sockaddr_in serveraddr, clientaddr;
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

	//使用select函数实现io多路复用
	
	//注意：当select函数返回之后，会移除集合里面当前文件描述符以外其他所有的文件描述符	
	fd_set readfds;
	int maxfd;

	//第一步：将集合清空
	FD_ZERO(&readfds);

	maxfd = sockfd;

	while(1)
	{
		//第二步：将需要的文件描述符添加到集合里面
		// 0 sockfd
		FD_SET(0, &readfds);
		FD_SET(sockfd, &readfds);

		//第三步：调用select函数阻塞等待文件描述符准备就绪
		if(select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0)
		{
			errlog("fail to select");
		}

		//根据性质判断到底是哪个文件描述符准备就绪，并执行相应的io操作
		
		if(FD_ISSET(sockfd, &readfds))
		{
			if((acceptfd = accept(sockfd, (struct sockaddr *)&clientaddr, &addrlen)) < 0)
			{
				errlog("fail to accept");
			}

			//打印获取到的客户端的信息
			printf("ip: %s, port: %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
            
            close(acceptfd);
		}
        
		if(FD_ISSET(0, &readfds))
		{
			if(NULL != fgets(buf, N, stdin))
			{
                buf[strlen(buf) - 1] = '\0';

			    printf("buf : %s\n", buf);
			}
    	}
 
        if(0 == strncasecmp(buf, "quit", 4))
             {
                 printf("server is quit!\n");
                 break;
             }
	}

	close(sockfd);

	return 0;
}

