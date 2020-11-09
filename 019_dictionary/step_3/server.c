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

void do_client(int acceptfd, sqlite3 *db);
void do_register(int acceptfd, MSG *msg, sqlite3 *db);
void do_login(int acceptfd, MSG *msg, sqlite3 *db);
void do_query(int acceptfd, MSG *msg, sqlite3 *db);
int  do_searchword(int acceptfd, MSG *msg);
void getdata(char data[]);

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
			do_register(acceptfd, &msg, db);
			break;
		case L :
			do_login(acceptfd, &msg, db);
			break;
		case Q :
			do_query(acceptfd, &msg, db);
			break;
		case H :
			//do_history();
			break;
		}
	}

	//对方退出或者异常关闭
	printf("client quit\n");
	exit(0);

	return;
}

//注册
void do_register(int acceptfd, MSG *msg, sqlite3 *db)
{
	char sqlstr[128] = {0};
	char *errmsg;

	sprintf(sqlstr, "insert into usr values('%s', '%s')", msg->name, msg->data);
	
	//调用函数在数据库里插入数据
	if(sqlite3_exec(db, sqlstr, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		//数据存在，插入失败
		sprintf(msg->data, "user %s already exist!!!", msg->name);
	}
	else
	{
		//插入成功
		strcpy(msg->data, "OK");
	}

	//将信息发送给客户端
	send(acceptfd, msg, sizeof(MSG), 0);

	return;
}

//登录
void do_login(int acceptfd, MSG *msg, sqlite3 *db)
{
	char sqlstr[128] = {0};
	char *errmsg, **result;
	int nrow, ncolumn;
	
	sprintf(sqlstr, "select * from usr where name = '%s' and pass = '%s'", msg->name, msg->data);
	
	//调用sqlite3_get_table查询数据是否在数据库的表里面
	if(sqlite3_get_table(db, sqlstr, &result, &nrow, &ncolumn, &errmsg) != SQLITE_OK)
	{
		printf("error : %s\n", errmsg);
	}
	
	//nrow为0表示数据不存在或者不匹配
	if(nrow == 0)
	{
		strcpy(msg->data, "name or password is wrony!!!");
	}
	else  //数据匹配
	{
		strcpy(msg->data, "OK");
	}

	//发送指令
	send(acceptfd, msg, sizeof(MSG), 0);
	
	return;
}

//查询
void do_query(int acceptfd, MSG *msg, sqlite3 *db)
{
	char sqlstr[128], *errmsg;
	int found = 0;
	char date[128], word[128];
	
	strcpy(word, msg->data);
	
	//查询成功返回1
	found = do_searchword(acceptfd, msg);

	//如果找到单词的解释，需要将单词以及时间保存在历史记录的数据库表里
	if(found == 1)
	{
		//时间
		getdata(date);
		
		sprintf(sqlstr, "insert into record values('%s', '%s', '%s')", msg->name, date, word);
		
		//将用户名、时间以及单词保存在历史记录表里
		if(sqlite3_exec(db, sqlstr, NULL, NULL, &errmsg) != SQLITE_OK)
		{
			printf("error : %s\n", errmsg);
		}
	}
	else
	{
		strcpy(msg->data, "not found");
	}

	send(acceptfd, msg, sizeof(MSG), 0);
	
	return;
}

//查找单词
int  do_searchword(int acceptfd, MSG *msg)
{
	FILE *fp;
	char temp[300];
	char *p;
	int len, result;
	
	//得到要查找单词的长度
	len = strlen(msg->data);

	//打开文档
	if((fp = fopen("dict.txt", "r")) == NULL)
	{
		strcpy(msg->data, "dict can not open");
		send(acceptfd, msg, sizeof(MSG), 0);
	}

	//printf("query word is %s len = %d\n", msg->data, len);
	
	//fgets每次只可以读一行，规定文档里面最后一行有300个字节
	while(fgets(temp, 300, fp) != NULL)
	{
		result = strncmp(msg->data, temp, len);

		//输入的单词必须保证一样，并且temp的下一个字符是空格
		if(result == 0 && temp[len] == ' ')
		{
			//指针指向单词后面的位置
			p = temp + len;

			//从第一个有数据的位置开始读，排除单词与解释之间的空格
			while(*p == ' ')
			{
				p++;
			}

			strcpy(msg->data, p);

			fclose(fp);

			return 1;
		}
	}

	//如果没找到
	fclose(fp);
	return 0;
}

//获取时间
void getdata(char data[])
{
	time_t t;
	struct tm *tp;
	time(&t);
	tp = localtime(&t);
	
	//将数据保存在data里面
	sprintf(data, "%d-%d-%d %d:%d:%d", 
		tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday, 
		tp->tm_hour, tp->tm_min, tp->tm_sec);

	return ;
}
