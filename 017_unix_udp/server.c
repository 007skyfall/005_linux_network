#include "net.h"

int main(int argc, const char *argv[])
{
	int sockfd;
	struct sockaddr_un serveraddr, clientaddr;
	socklen_t addrlen = sizeof(serveraddr);
	char buf[N] = { 0 };
	ssize_t bytes;

	//第一步：创建套接字
	if((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
	{
		errlog("fail to socket");
	}
    remove(argv[1]);//删除套接字  自己
	//第二步：填充服务器本地信息结构体
	serveraddr.sun_family = AF_UNIX;
	strcpy(serveraddr.sun_path, argv[1]);

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
			if(strncasecmp(buf, "quit", 4) == 0)
			{
				printf("client is quited\n");
				break;
			}
			else 
			{
				printf("clientaddr.sun_path = %s\n", clientaddr.sun_path);
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

