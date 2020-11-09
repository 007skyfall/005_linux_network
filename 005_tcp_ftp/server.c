#include "net.h"

void do_list(int acceptfd)
{
	DIR *dirp;
	struct dirent *dir;
	char buf[N] = { 0 };

	//打开目录
	if((dirp = opendir(".")) == NULL)
	{
		errlog("fail to opendir");
	}
	
	//获取到目录中的文件名
	while((dir = readdir(dirp)) != NULL)
	{	
		if(dir->d_name[0] == '.')
		{
			continue;
		}
        memset(buf, 0, sizeof(buf));
		//将文件名发送给客户端
		strcpy(buf, dir->d_name);
		send(acceptfd, buf, N, 0);
	}

	//发送结束标志
	strcpy(buf, "**OVER**");
	send(acceptfd, buf, N, 0);

	printf("文件名发送完毕\n");

	return ;
}

int do_download(int acceptfd, char *filename)
{
	int fd;
	char buf[N] = { 0 };
	ssize_t bytes;

    memset(buf, 0, sizeof(buf));

	//打开文件，判断文件是否存在
	if((fd = open(filename, O_RDONLY)) < 0)
	{
		//如果文件不存在，告知客户端
		if(errno == ENOENT)
		{
			strcpy(buf, "NO");
			send(acceptfd, buf, N, 0);

			return -1;
		}
		else
		{
			errlog("fail to open");
		}
	}

	//如果文件存在，告知客户端
	strcpy(buf, "YES");
	send(acceptfd, buf, N, 0);
	
	//读取文件内容并发送
	while((bytes = read(fd, buf, N)) > 0)
	{
		send(acceptfd, buf, bytes, 0);
	}

	//解决数据粘包问题
	sleep(1);

	strcpy(buf, "**OVER**");
	send(acceptfd, buf, N, 0);

	printf("文件内容发送完毕\n");

	close(fd);
    
	return 0;
}

int do_upload(int acceptfd, char *filename)
{
	int fd;
	char buf[N] = { 0 };
	ssize_t bytes;
    
    memset(buf, 0, sizeof(buf));

    //创建文件
	if((fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0664)) < 0)
	{
		errlog("fail to open");
	}
	
	//接收数据并写入文件
	while((bytes = recv(acceptfd, buf, N, 0)) > 0)
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

	printf("文件接收完毕\n");

	close(fd);
    
	return 0;
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

	while(1)
	{
		//第五步：阻塞等待客户端的连接请求
		if((acceptfd = accept(sockfd, (struct sockaddr *)&clientaddr, &addrlen)) < 0)
		{
			errlog("fail to accept");
		}

		//打印获取到的客户端的信息
		printf("ip: %s, port: %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		while(1)
		{
			//接收数据并处理
			if((bytes = recv(acceptfd, buf, N, 0)) < 0)  //L G P
			{
				errlog("fail to recv");
			}
			else if(bytes == 0)
			{
				printf("Client is quited!!!\n");
				break;
			}
			else
			{
				printf("buf = %s\n", buf);

				switch(buf[0])
				{
				case 'L':
					do_list(acceptfd);
					break;
				case 'G':  //G filename
					do_download(acceptfd, buf + 2);
					break;
				case 'P':
					do_upload(acceptfd, buf + 2);
					break;
				}
			}
		}
        
        if(bytes == 0)
        {
          break;
        }
	}

	close(acceptfd);
	close(sockfd);

	return 0;
}

