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
#define LOGIN  1
#define CHAT   2
#define LOGOUT 3



typedef struct {
   char type;     //消息类型
   char name[32]; //登录用户名
   char msg[128]; //消息内容 
} Message_t;

struct sockaddr_in serverAddr;
char name[32];

void *sendMsg(int Fd){
  int selen;
  char buf[RECEIVE_BUFF_LEN];
  int len;
  Message_t msgs={0};
  
  len = sizeof(serverAddr);
  while(1){

      scanf("%s",msgs.msg);
      strcpy(msgs.name,name);
      if(strcmp(msgs.msg,"bye")== 0)
      {
          msgs.type = LOGOUT;  
          
          selen =  sendto(Fd, &msgs, sizeof(Message_t), 0,(struct sockaddr *)&serverAddr, len);
          exit(0);
      }else{
      
        msgs.type = CHAT; 
        selen =  sendto(Fd, &msgs, sizeof(Message_t), 0,(struct sockaddr *)&serverAddr, len);
      }
      
      
  } 

}

int main(int argc, char **argv){

// 1 定义socket 
// int socket(int domain, int type, int protocol); 
//  domain: AF_UNIX / AF_INET
//  type:  SOCK_DGRAM
// protocol: 0
   int udpClientFd;
   int ret;
   int len;
   int relen;
   int selen;
   Message_t msgs;
   pthread_t sendthread;

   char buf[RECEIVE_BUFF_LEN] = {0};
   
   struct sockaddr_in clientAddr;
   len = sizeof(serverAddr);
   
   if(argc!=4){

       printf("Usage: %s  serverIp sPort name\n",argv[0]);
       return -1;

   }
   
   udpClientFd = socket(AF_INET, SOCK_DGRAM, 0);
   if(udpClientFd < 0){
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

#if 0
// 3 绑定地址  int bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen); 
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(atoi(argv[2]));
    clientAddr.sin_addr.s_addr =  inet_addr(argv[1]);

   ret = bind(udpClientFd,(struct sockaddr *)&clientAddr,len);
   if(ret<0){
      perror("Failed to bind:");
      return -1;
   }

#endif



   //只要客户端上线，马上给服务器发送消息，告诉服务器自己的名字
   strcpy(name,argv[3]);
   
   strcpy(msgs.msg,name);
   strcpy(msgs.name,name);
   msgs.type = LOGIN;
   sendto(udpClientFd, &msgs, sizeof(Message_t), 0,(struct sockaddr *)&serverAddr, len); //发送登录消息
                
   pthread_create(&sendthread, NULL,sendMsg, (void*)udpClientFd); //创建发送消息线程
  

// 4 发送数据接收数据
   
   while(1){


//      scanf("%s",buf);
//      selen =  sendto(udpClientFd, buf, strlen(buf), 0,(struct sockaddr *)&serverAddr, len);
       memset(buf,0,RECEIVE_BUFF_LEN);
       relen =  recvfrom(udpClientFd, buf, RECEIVE_BUFF_LEN, 0,
                            (struct sockaddr *)&clientAddr, &len); 
       if(relen>0){

           
            printf("%s",buf);
          //printf("%s ,port=%d,ip=%s\n",buf,ntohs(clientAddr.sin_port),inet_ntoa(clientAddr.sin_addr));   
          //memset(buf,0,RECEIVE_BUFF_LEN);
    /*      strcpy(buf,"udp server send message\n");
          selen =  sendto(udpServerFd, buf, strlen(buf), 0,
                       (struct sockaddr *)&clientAddr, len);
    */                   
                       
           
       }else{
          perror("Failed to recvfrom:");
          close(udpClientFd);
          return -1;
       }

   } 

//

// 5 关闭 连接 
   close(udpClientFd);

   return 0;

}