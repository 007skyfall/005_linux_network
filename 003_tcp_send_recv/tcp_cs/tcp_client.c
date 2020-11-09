
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
	
	/* 定义一个客户端socket文件描述符*/
	int clientFd;
	int len;
	char buff[100];
	int ret;       /* 返回值 */ 
	struct sockaddr_in serverAdd,clientAdd;   
	len=sizeof(serverAdd);
	
     if(argc!=3){
          printf("Usage:%s ipaddr port\n",argv[0]);  
          return -1;
     }
    

	
	/* 定义在/usr/include下面，可以搜索 */
	
	clientFd = socket(AF_INET,SOCK_STREAM,0); /* 定义socket */
	/* 
	 第一个参数：网络类型   ipv4 AF_INET，
						    ipv6 AF_INET6
							unix AF_UNIX
	 第二个参数：tcp SOCK_STREAM
	             UDP  SOCK_DGRAM
				 
	 第三个参数：通常填0
	 
 	*/
	if(clientFd<0){
		printf("Failed to create socket \n");
		return -1;
	}
	
	clientAdd.sin_family = AF_INET;    /* 网络类型    */
	clientAdd.sin_port = htons(9999);  /* 端口注意字节序转换  ，本机通信注意端口号不要和服务器相同*/
	clientAdd.sin_addr.s_addr = 0     ;  /* IP地址要转换成32位网络地址*/
	                                     // 0.0.0.0 地址表示任意IP地址，转换为32位IP地址就是0
	
	ret = bind(clientFd,(struct sockaddr *)&clientAdd,len);
	/* 第一个参数是我们生成的fd，第二个参数注意强制类型转换 */
	if(ret<0){
		printf("Failed to bind server addr \n");
		return -1;
	}

	
	serverAdd.sin_family = AF_INET;    /* 网络类型*/
	serverAdd.sin_port = htons(atoi(argv[2]));  /* 端口注意字节序转换 */
	serverAdd.sin_addr.s_addr = inet_addr(argv[1]);  /* IP地址要转换成32位网络地址*/

	ret = connect(clientFd,(struct sockaddr *)&serverAdd,len);

    while(1){

        //strcpy(buff,"Hello,I am a client!\n");
        printf("wait input data\n");
        fgets(buff,100,stdin);
     	ret = send(clientFd, buff, strlen(buff), 0);
        if(ret <0){
	       perror("send error:");
	       return -1;

        }
        printf("wait server data\n");
        memset(buff,0,100);
        ret = read(clientFd,buff,100);
        
        if(ret <= 0){                //读小于等于0都退出，关闭连接
	       perror("read error:");
	       close(clientFd);
	       return -1;

        }else{
            printf("%s\n",buff);

        }
	}

	close(clientFd);
	
	
}
