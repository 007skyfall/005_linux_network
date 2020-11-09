#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc,char **argv)
{
	
	int serverFd;  
	
	/* 定义一个服务器socket文件描述符 */
	int clientFd;
	char buff[100];
	int ret;       /* 返回值 */ 
	struct sockaddr_in serverAdd,clientAdd;  
	int len=sizeof(serverAdd);
	/* 定义在/usr/include下面，可以搜索 */
	 if(argc!=3){
          printf("Usage:%s ipaddr port\n",argv[0]);  
          return -1;
     }
	serverFd = socket(AF_INET,SOCK_STREAM,0); /* 定义socket */
	if(serverFd<0){
		printf("Failed to create socket \n");
		return -1;
	}
	
	serverAdd.sin_family = AF_INET;    /* 网络类型*/
	serverAdd.sin_port = htons(atoi(argv[2]));  /* 端口注意字节序转换*/
	serverAdd.sin_addr.s_addr = inet_addr(argv[1]);   /* IP地址要转换成32位网络地址*/
	
	ret = bind(serverFd,(struct sockaddr *)&serverAdd,sizeof(serverAdd));
	/* 第一个参数是我们生成的fd，第二个参数注意强制类型转换 */
	if(ret<0){
		printf("Failed to bind server addr \n");
		return -1;
	}
	
	ret = listen(serverFd,10);  /* 注意第二个参数是 客户未成功连接队列的数量，一般按需要填写 */
	if(ret<0){
		printf("Failed to listen \n");
		return -1;
	}
	
	while(1){
		
		clientFd = accept(serverFd,(struct sockaddr *)&clientAdd,&len);
		
		
		if(clientFd<0){
			perror("Failed to accept \n");
			return -1;
		}
		while(1){	
		    memset(buff,0,100);

		    printf("wait client data\n");
		    ret = read(clientFd,buff,100);  //read函数在客户端的socket关闭时候会不阻塞，一直返回0
		                                    //所以必须要加以判断，防止服务器死循环。
		    if(ret>0){
		        printf("receive client data:%s\n",buff);
		        memset(buff,0,100);

		        printf("wait input data\n");
		        fgets(buff,100,stdin);
		        ret  = send(clientFd, buff, strlen(buff), 0);
		        if(ret <0){
	                perror("send error:");
	                close();
	                break;
                }
		        
		    }else{
                break;
		    }

		    
		    
		}
		close(clientFd);
		
		
	}
	close(clientFd);
	
}