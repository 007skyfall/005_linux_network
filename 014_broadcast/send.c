#include "net.h"

int main(int argc, const char *argv[])
{
	int sockfd;
	struct sockaddr_in broadcastaddr;
	socklen_t addrlen = sizeof(broadcastaddr);
	char buf[N] = { 0 };

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

	//第二步：填充广播信息结构体
	//inet_addr：将点分十进制ip地址转化为网络字节序的整型数据
	//htons：将主机字节序转化为网络字节序
	//atoi：将数字型字符串转化为整型数据
	broadcastaddr.sin_family = AF_INET;
	broadcastaddr.sin_addr.s_addr = inet_addr(argv[1]);  //192.168.2.255 255.255.255.255
	broadcastaddr.sin_port = htons(atoi(argv[2]));

	//设置为发送广播权限
	int on = 1;

	if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0)
	{
		errlog("fail to setsockopt");
	}

	while(1)
	{
		if(!fgets(buf, N, stdin))
		{
		    buf[strlen(buf) - 1] = '\0';
		}
		if(sendto(sockfd, buf, N, 0, (struct sockaddr *)&broadcastaddr, addrlen) < 0)
		{
			errlog("fail to sendto");
		}

	}

	close(sockfd);

	return 0;
}

