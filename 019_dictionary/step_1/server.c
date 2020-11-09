#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define  N  16
#define  R  1   //  user register
#define  L  2   //  user login
#define  Q  3   //  query word
#define  H  4   //  history record

//定义数据库
#define DATABASE "my.db"

typedef struct 
{
	int type;
	char name[N];
	char data[256];   // password or word
} MSG;

int main(int argc, char *argv[])
{
	int sockfd, acceptfd;
	struct sockaddr_in server_addr;
	pid_t pid;
	sqlite3 *db;

	if (argc < 3)
	{
		printf("Usage : %s <ip> <port>\n", argv[0]);
		exit(-1);
	}
	
	//打开数据库（如果数据库已经创建好了，调用函数后，之后利用指针操作数据库）
	//数据库里面有两个表，一个负责存放用户名和密码，用户名唯一，另一个负责存放历史记录
	if (sqlite3_open(DATABASE, &db) != SQLITE_OK)
	{
		printf("error : %s\n", sqlite3_errmsg(db));
		exit(-1);
	}
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("fail to socket");
		exit(-1);
	}

	bzero(&server_addr, sizeof(server_addr));
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));
	
	if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("fail to bind");
		exit(-1);
	}
	
	if (listen(sockfd, 5) < 0)
	{
		perror("fail to listen");
		exit(-1);
	}
	
	signal(SIGCHLD, SIG_IGN);// 避免僵尸进程
	
	while(1)
	{
		//不关心客户端的网络信息结构体
		if((acceptfd = accept(sockfd, NULL, NULL)) < 0)
		{
			perror("fail to accept");
			exit(-1);
		}
	
		//父子进程实现并发
		if((pid = fork()) < 0)
		{
			perror("fail to fork");
			exit(-1);
		}
		if(pid == 0)  //子进程负责接收数据并处理
		{
			close(sockfd);
			do_client(acceptfd, db);
		}
		else  //父进程负责连接
		{
			close(acceptfd);
		}
	}

	return 0;
}

void do_client(int acceptfd, sqlite3 *db)
{
	MSG msg;

	//根据接收到的type判断对应执行的代码
	while (recv(acceptfd, &msg, sizeof(MSG), 0) > 0)  // receive request
	{
		printf("type : %d, name : %s, data : %s\n", msg.type, msg.name, msg.data);

		switch ( msg.type )
		{
		case R :
			do_register();
			break;
		case L :
			do_login();
			break;
		case Q :
			do_query();
			break;
		case H :
			do_history();
			break;
		}
	}

	//对方退出或者异常关闭
	printf("client quit\n");
	exit(0);

	return;
}

