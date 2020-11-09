#include <stdio.h>  //printf
#include <arpa/inet.h> //inet_addr htons
#include <sys/types.h>
#include <sys/socket.h>  //socket bind listen accept connect
#include <netinet/in.h>  //sockaddr_in
#include <stdlib.h> //exit
#include <unistd.h> //close
#include <string.h>

#define N 128
#define errlog(errmsg) do{\
							perror(errmsg);\
							printf("%s -- %s -- %d\n", __FILE__, __func__, __LINE__);\
							exit(1);\
						 }while(0)
#define L 1
#define C 2
#define Q 3

typedef struct{
	int type;
	char name[N];
	char text[N];
}MSG;

typedef struct node{
	struct sockaddr_in addr;
	struct node *next;
}linklist_t;

linklist_t *linklist_create();
void do_login(int sockfd, MSG msg, linklist_t *h, struct sockaddr_in clientaddr);
void do_chat(int sockfd, MSG msg, linklist_t *h, struct sockaddr_in clientaddr);
void do_quit(int sockfd, MSG msg, linklist_t *h, struct sockaddr_in clientaddr);
int main(int argc, const char *argv[])
{
	int sockfd;
	struct sockaddr_in serveraddr, clientaddr;
	socklen_t addrlen = sizeof(serveraddr);
	MSG msg;

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

	//创建父子进程实现一边发送系统信息，一边接收数据
	pid_t pid;

	if((pid = fork()) < 0)
	{
		errlog("fail to fork");
	}
	else if(pid == 0)  //子进程负责发送系统信息
	{
		msg.type = C;
		strcpy(msg.name, "server");

		while(1)
		{
			fgets(msg.text, N, stdin);
			msg.text[strlen(msg.text) - 1] = '\0';

			sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&serveraddr, addrlen);
		}
	}
	else //父进程负责接收数据并处理
	{
		linklist_t *h = linklist_create();

		while(1)
		{
			recvfrom(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&clientaddr, &addrlen);

			printf("%d --> %s --> %s\n", msg.type, msg.name, msg.text);

			switch(msg.type)
			{
			case L:
				do_login(sockfd, msg, h, clientaddr);
				break;
			case C:
				do_chat(sockfd, msg, h, clientaddr);
				break;
			case Q:
				do_quit(sockfd, msg, h, clientaddr);
				break;
			}
		}
	}

	return 0;
}

linklist_t *linklist_create()
{
	linklist_t *h = (linklist_t *)malloc(sizeof(linklist_t));
	h->next = NULL;

	return h;
}

void do_login(int sockfd, MSG msg, linklist_t *h, struct sockaddr_in clientaddr)
{
	//将某人登录的信息发送给其他所有在线的用户
	sprintf(msg.text, "-------------------- %s login ---------------------", msg.name);
	
	//遍历链表，向数据域发送数据
	linklist_t *p = h;

	while(p->next != NULL)
	{
		sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&p->next->addr, sizeof(struct sockaddr_in));

		p = p->next;
	}

	//把当前登录的新用户的信息保存在链表里面
	linklist_t *temp = (linklist_t *)malloc(sizeof(linklist_t));
	temp->addr = clientaddr;

	temp->next = h->next;
	h->next = temp;
	
	return ;
}

void do_chat(int sockfd, MSG msg, linklist_t *h, struct sockaddr_in clientaddr)
{
	char buf[N] = {};
	linklist_t *p = h->next;

	//将某人说的话发送给其他人
	sprintf(buf, "%s : %s", msg.name, msg.text);
	strcpy(msg.text, buf);

	while(p != NULL)
	{
		//自己不接收自己发送的数据
		if(memcmp(&p->addr, &clientaddr, sizeof(clientaddr)) == 0)
		{
			p = p->next;
		}
		else 
		{
			sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&p->addr, sizeof(struct sockaddr_in));

			p = p->next;
		}
	}

	return ;
}

void do_quit(int sockfd, MSG msg, linklist_t *h, struct sockaddr_in clientaddr)
{
	//将某人下线的信息发送给其他在线的人
	sprintf(msg.text, "-------------------- %s offline ---------------------", msg.name);
	
	linklist_t *p = h;
	linklist_t *temp;

	while(p->next != NULL)
	{
		//如果找到自己的信息，删除这个结点
		if(memcmp(&p->next->addr, &clientaddr, sizeof(clientaddr)) == 0)
		{
			temp = p->next;
			p->next = temp->next;

			free(temp);
			temp = NULL;
		}
		else 
		{
			sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&p->next->addr, sizeof(struct sockaddr_in));

			p = p->next;
		}
	}

	return ;
}
