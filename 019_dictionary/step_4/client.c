#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define  N  16
#define  R  1   //  注册
#define  L  2   //  登录
#define  Q  3   //  查询单词
#define  H  4   //  查询历史记录

#define DATABASE "my.db"

typedef struct 
{
	int type;
	char name[N];
	char data[256];   // password or word or remark
} MSG;

void do_register(int sockfd, MSG *msg);
int do_login(int sockfd, MSG *msg);
void do_query(int sockfd, MSG *msg);
void do_history(int sockfd, MSG *msg);

int main(int argc, char *argv[])
{
	int sockfd ;
	struct sockaddr_in server_addr;
	MSG msg;
	if (argc < 3)
	{
		printf("Usage : %s <serv_ip> <serv_port>\n", argv[0]);
		exit(-1);
	}

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("fail to socket");
		exit(-1);
	}

	bzero(&server_addr, sizeof(server_addr));
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));
	
	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("fail to connect");
		exit(-1);
	}
	
	int n;
	while(1)
	{
		printf("************************************\n");
		printf("* 1: register   2: login   3: quit *\n");
		printf("************************************\n");
		printf("please choose : ");

		if(scanf("%d", &n) <= 0)
		{
			perror("scanf");
			exit(-1);
		}

		switch(n)
		{
			case 1:
				do_register(sockfd, &msg);
				break;
			case 2:
				if(do_login(sockfd, &msg) == 1)
				{
					goto next;
				}
				break;
			case 3:
				close(sockfd);
				exit(1);
		}
	}
next:
	while(1)
	{
		printf("************************************\n");
		printf("* 1: query   2: history   3: quit *\n");
		printf("************************************\n");
		printf("please choose : ");

		if(scanf("%d", &n) <= 0)
		{
			perror("scanf");
			exit(-1);
		}

		switch(n)
		{
			case 1:
				do_query(sockfd, &msg);
				break;
			case 2:
				do_history(sockfd, &msg);
				break;
			case 3:
				close(sockfd);
				exit(0);
		}
	}
	return 0;
}

//注册
void do_register(int sockfd, MSG *msg)
{
	//输入指令以及用户名和密码，发送给服务器
	msg->type = R;
	
	printf("input your name:");
	scanf("%s", msg->name);
	
	printf("input your password:");
	scanf("%s", msg->data);
	
	send(sockfd, msg, sizeof(MSG), 0);
	
	//接收数据，判断是否注册成功
	recv(sockfd, msg, sizeof(MSG), 0);
	
	printf("register : %s\n", msg->data);
	
	return;
}

//登录
int do_login(int sockfd, MSG *msg)
{
	//输入指令以及用户名和密码，发送给服务器
	msg->type = L;
	
	printf("input your name:");
	scanf("%s", msg->name);
	
	printf("input your password:");
	scanf("%s", msg->data);
	
	send(sockfd, msg, sizeof(MSG), 0);
	
	//接收数据，判断是否登录成功
	recv(sockfd, msg, sizeof(MSG), 0);
	
	//如果登录成功，返回1
	if(strncmp(msg->data, "OK", 2) == 0)
	{
		printf("login : OK\n");
		return 1;
	}

	//登录失败返回0
	printf("login : %s\n", msg->data);
	
	return 0;
}

//查询单词
void do_query(int sockfd, MSG *msg)
{
	//告知服务器查询单词
	msg->type = Q;
	puts("---------");

	while(1)
	{
		printf("input word : ");
		scanf("%s", msg->data);
	
		//如果输入#，回退到上一层
		if(strcmp(msg->data, "#") == 0)
		{
			break;
		}

		//发送指令和单词
		send(sockfd, msg, sizeof(MSG), 0);
		
		//接收数据
		recv(sockfd, msg, sizeof(MSG), 0);
		
		putchar(10);
		printf("********************************************************************************\n");
		printf("%s\n", msg->data);
		printf("********************************************************************************\n");
		putchar(10);
	}
	
	return;
}

//查询历史记录
void do_history(int sockfd, MSG *msg)
{
	//发送指令告知服务器查询历史记录
	msg->type = H;
	send(sockfd, msg, sizeof(MSG), 0);

	while(1)
	{
		//接收数据必并打印
		recv(sockfd, msg, sizeof(MSG), 0);
		if(msg->data[0] == '\0')
		{
			break;
		}
		
		printf("%s\n", msg->data);
	}

	return;
}
