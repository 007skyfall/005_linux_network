#include "net.h"

void do_help(void)
{
	printf("*********************************************************\n");
	printf("*****           输入 / 功能           *******************\n");
	printf("*****           list / 查看服务器所在目录的文件名   *****\n");
	printf("*****   get filename / 下载服务器所在目录的文件  ********\n");
	printf("*****   put filename / 上传文件到服务器  ****************\n");
	printf("*****           quit / 退出           *******************\n");
	printf("*********************************************************\n");

	return ;
}

void do_list(int sockfd)
{
	char buf[N] = { 0 };

	//发送指令告知服务器指向list功能
	//buf[0] = 'L';
	strcpy(buf, "L");
	send(sockfd, buf, N, 0);

	//接收服务器发送的数据并打印
	while(1)
	{
		recv(sockfd, buf, N, 0);

		if(strncmp(buf, "**OVER**", 8) == 0)
		{
			break;
		}

		printf("### %s\n", buf);
	}

	printf("文件名接收完毕\n");

	return ;
}

int do_download(int sockfd, char *filename)
{
	int fd;
	char buf[N] = { 0 };
	ssize_t bytes;

	//发送指令以及文件名告知服务器执行下载功能
	sprintf(buf, "G %s", filename);
	send(sockfd, buf, N, 0);

	//接收数据判断文件是否存在
	recv(sockfd, buf, N, 0);
	
	//如果文件不存在，打印提示语句重新输入
	if(strncmp(buf, "NO", 2) == 0)
	{
		printf("文件%s不存在，请重新输入\n", filename);
		return -1;
	}
	
	//如果文件存在，创建文件
	if((fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC,0664)) < 0)
	{
		errlog("fail to open");
	}
	
	//接收数据并写入文件
	while((bytes = recv(sockfd, buf, N, 0)) > 0)
	{
		if(strncmp(buf, "**OVER**", 8) == 0)
		{
			break;
		}

		if(write(fd, buf, bytes) < 0)
        {
            errlog("fail to write");
        }
	}

	printf("文件下载完毕\n");

	close(fd);
	return 0;
}

int do_upload(int sockfd, char *filename)
{
	int fd;
	char buf[N] = { 0 };
	ssize_t bytes;

	//打开文件，判断文件是否存在
	if((fd = open(filename, O_RDONLY)) < 0)
	{
		//如果文件不存在，则退出函数重新输入
		if(errno == ENOENT)
		{
			printf("文件%s不存在，请重新输入\n", filename);
			return -1;
		}
		else 
		{
			errlog("fail to open");
		}
	}

	//如果文件存在，发送指令以及文件名，执行上传操作
	sprintf(buf, "P %s", filename);
	send(sockfd, buf, N, 0);

	//读取文件内容并发送
	while((bytes = read(fd, buf, N)) > 0)
	{
		send(sockfd, buf, bytes, 0);
	}

	sleep(1);

	strcpy(buf, "**OVER**");
	send(sockfd, buf, N, 0);


	printf("文件上传完毕\n");

	close(fd);
	return 0;
}

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

	//第三步：发送客户端的连接请求
	if(connect(sockfd, (struct sockaddr *)&serveraddr, addrlen) < 0)
	{
		errlog("fail to connect");
	}

	printf("****************************\n");
	printf("*** 请输入 help 查看选项 ***\n");
	printf("****************************\n");

	while(1)
	{
		printf("input >>> ");

		if(NULL == fgets(buf, N, stdin))  //help list get+filename put+filename quit
		{
            fprintf(stderr, "fgets error!\n");
            continue;
		}
        buf[strlen(buf) - 1] = '\0';

		//根据输入的内容作出相应的判断
		if(strncasecmp(buf, "help", 4) == 0)
		{
			do_help();
		}
		else if(strncasecmp(buf, "list", 4) == 0)
		{
			do_list(sockfd);
		}
		else if(strncasecmp(buf, "get", 3) == 0)  //get filename
		{
			do_download(sockfd, buf + 4);
		}
		else if(strncasecmp(buf, "put", 3) == 0)
		{
			do_upload(sockfd, buf + 4);
		}
		else if(strncasecmp(buf, "quit", 4) == 0)
		{
			printf("The client is quited\n");
			break;
		}
		else 
		{
			printf("您输入的有误，请重新输入!!!\n");
		}
	}
	
	close(sockfd);
	
	return 0;
}

