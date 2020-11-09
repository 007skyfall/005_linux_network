#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define RECEIVE_BUFF_LEN 1024

#define MAX_CLI_COUNT 100  //最大用户数

struct sockaddr_in serverAddr;
int addlen = sizeof(struct sockaddr_in);

typedef struct {
   
   char used;
   struct sockaddr_in cAddr;
} AClAdr;   //地址表结构


AClAdr AllCliAddrs[MAX_CLI_COUNT]; //登录的客户端地址表


#define LOGIN  1
#define CHAT   2
#define LOGOUT 3


typedef struct {
    char type;     //消息类型
    char name[32]; //登录用户名
    char msg[128]; //消息内容 
} Message_t;


void *sendMsg(int Fd){   //服务器端发送消息函数
  int selen;
  //char buf[RECEIVE_BUFF_LEN];
  Message_t msgt;
  int len;
  len = sizeof(serverAddr);
  while(1){

      scanf("%s",msgt.msg);
      msgt.type = CHAT;
      strcpy(msgt.name,"server");
      selen =  sendto(Fd, &msgt, sizeof(msgt), 0,(struct sockaddr *)&serverAddr, len);
  } 

}


void *recvMsg(int fd)
{

   struct sockaddr_in clientAddr;
   Message_t msgt;
   char buf[128];
   int i;
   int msglen = sizeof(Message_t); 
    while(1)
	{
	  //调用recvfrom函数接收客户端的消息
	  memset(&msgt,0,sizeof(Message_t));
      recvfrom(fd, &msgt,sizeof(msgt) , 0,(struct sockaddr *)&clientAddr, &addlen); 
      //printf("t%d,%s,%s ,port=%d,ip=%s\n",msgt.type,msgt.name,msgt.msg,ntohs(clientAddr.sin_port),inet_ntoa(clientAddr.sin_addr)); 
       switch(msgt.type){

       case LOGIN://  1 处理用户上线消息，通知其他用户
       {
            for(i=0;i<MAX_CLI_COUNT;i++){
                if(!AllCliAddrs[i].used){
                   
                    memcpy(&AllCliAddrs[i].cAddr,&clientAddr,addlen);
                    AllCliAddrs[i].used = 1;
                    break;             //找到一个空的未用的地址，填充login的用户地址，然后跳出
                }

            }
            sprintf(buf,"%s login\n",msgt.name); 
            for(i=0;i<MAX_CLI_COUNT;i++){   //给所有除自己的在线的用户群发消息

               if(AllCliAddrs[i].used && memcmp(&AllCliAddrs[i].cAddr,
                            &clientAddr,sizeof(clientAddr))!=0){ 
                  
                  sendto(fd, buf, strlen(buf), 0,(struct sockaddr *)&AllCliAddrs[i].cAddr, addlen);
               }
            }
            printf("%s",buf); 
            break;
    
       }
       case LOGOUT://  2 处理用户下线消息，通知其他用户
       {

       
           for(i=0;i<MAX_CLI_COUNT;i++){     //用户下线，把该用户地址从地址表设置为未用状态
                if(AllCliAddrs[i].cAddr.sin_addr.s_addr == clientAddr.sin_addr.s_addr &&
                    AllCliAddrs[i].cAddr.sin_port == clientAddr.sin_port)
                {
                    //memset(AllCliAddrs[i].cAddr,0,addlen);
                    AllCliAddrs[i].used = 0;
                    break;
                }
           
            }
           sprintf(buf,"%s logout\n",msgt.name); 
           
           for(i=0;i<MAX_CLI_COUNT;i++){

               if(AllCliAddrs[i].used && memcmp(&AllCliAddrs[i].cAddr,
                        &clientAddr,sizeof(clientAddr))!=0){ //给所有除自己的在线的用户群发消息
                  sendto(fd, buf, strlen(buf), 0,(struct sockaddr *)&AllCliAddrs[i].cAddr, addlen);
                  
               }

            }
           printf("%s",buf); 
           break;  

       }

       case CHAT: //  3 收到其他用户发来的消息再转发sendto给其他用户
       {
           sprintf(buf,"%s said:%s\n",msgt.name,msgt.msg);
           
           for(i=0;i<MAX_CLI_COUNT;i++){  //给所有除自己的在线的用户群发消息
           
                  if(AllCliAddrs[i].used && memcmp(&AllCliAddrs[i].cAddr,
                            &clientAddr,sizeof(clientAddr))!=0){ 
                    sendto(fd, buf, strlen(buf), 0,(struct sockaddr *)&AllCliAddrs[i].cAddr, addlen);
           
               }
             

          }
          printf("%s",buf); 
           break;
        
       }
       default:
          break;
       
	  
     }

   }
}
int main(int argc, char **argv){

// 1 定义socket 
// int socket(int domain, int type, int protocol); 
//  domain: AF_UNIX / AF_INET
//  type:  SOCK_DGRAM
// protocol: 0
   int udpServerFd;
   int ret;
   int len;
   int relen;
   int selen;
   pthread_t sendthread,recvThread;
   char name[32];
   char buf[RECEIVE_BUFF_LEN] = {0};
   
   
   len = sizeof(serverAddr);
   memset(&AllCliAddrs,0,sizeof(AClAdr)*MAX_CLI_COUNT);
   
   if(argc!=3){

       printf("Usage: %s  serverIp sPort\n",argv[0]);
       return -1;

   }
   
   udpServerFd = socket(AF_INET, SOCK_DGRAM, 0);
   if(udpServerFd < 0){
        perror("Failed to create socket:");
        return -1;
   }

// 2 填充地址
//  struct sockaddr_in 
// sin_family 填充 AF_INET 或者 AF_UNIX 
// sin_addr.s_addr 地址  sin_port 端口
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr =  inet_addr(argv[1]);
    serverAddr.sin_port = htons(atoi(argv[2]));


// 3 绑定地址  int bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen); 
//    clientAddr.sin_family = AF_INET;
//    clientAddr.sin_port = htons(atoi(argv[2]));
//    clientAddr.sin_addr.s_addr =  inet_addr(argv[1]);

   ret = bind(udpServerFd,(struct sockaddr *)&serverAddr,len);
   if(ret<0){
      perror("Failed to bind:");
      return -1;
   }

   pthread_create(&sendthread, NULL,sendMsg, (void*)udpServerFd);//创建发送线程

   pthread_create(&recvThread, NULL,recvMsg, (void*)udpServerFd); //创建接收线程

   while(1)
   { 
      sleep(1);   //主main函数休眠
      
   }


}
