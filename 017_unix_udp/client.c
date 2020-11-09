#include "net.h"

int main(int argc, const char *argv[])
{
	int sockfd;
	struct sockaddr_un serveraddr, clientaddr;
	socklen_t addrlen = sizeof(serveraddr);
	char buf[N] = { 0 };

	//第一步：创建套接字
	if((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
	{
		errlog("fail to socket");
	}
    remove(argv[1]);//删除套接字  自己
	//第二步：填充服务器本地信息结构体
	serveraddr.sun_family = AF_UNIX;    //指定unix域
	strcpy(serveraddr.sun_path, argv[1]);   //填充地址   套接字路径

	//指定本地的地址信息
	clientaddr.sun_family=AF_UNIX;//指定unix域
    strcpy(clientaddr.sun_path,argv[1]);//填充地址   套接字路径
    remove(argv[1]);//删除套接字  自己

	if(bind(sockfd, (struct sockaddr *)&clientaddr, addrlen) < 0)
	{
		errlog("fail to bind");
	}

	while(1)
	{
		if(!fgets(buf, N, stdin))
		{
		    buf[strlen(buf) - 1] = '\0';
		}
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

