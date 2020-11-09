#include "net.h"

int main(int argc, const char *argv[])
{
	int sockfd;
	struct sockaddr_un serveraddr;
	socklen_t addrlen = sizeof(serveraddr);
	char buf[N] = { 0 };

	//第一步：创建套接字
	if((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		errlog("fail to socket");
	}

	//第二步：填充服务器本地信息结构体
	serveraddr.sun_family = AF_UNIX;
	strcpy(serveraddr.sun_path, argv[1]);

	//第三步：发送客户端的连接请求
	if(connect(sockfd, (struct sockaddr *)&serveraddr, addrlen) < 0)
	{
		errlog("fail to connect");
	}
	
	while(1)
	{
		if(NULL !=fgets(buf, N, stdin))
		{
		    buf[strlen(buf) - 1] = '\0';
		}
		if(send(sockfd, buf, N, 0) < 0)
		{
			errlog("fail to send");
		}

		if(strncasecmp(buf, "quit", 4) == 0)
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

			printf("from server : %s\n", buf);
		}
	}
	
	close(sockfd);
	
	return 0;
}

