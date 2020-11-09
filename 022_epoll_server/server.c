#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc,char **argv){
	
	int serverFd,serverFd2,serverFd3;  
	
	/* 定义一个服务器socket文件描述符*/
	int clientFd,i;
	int epfd;
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

//    serverFd2 = socket(AF_INET,SOCK_STREAM,0);

//    close(serverFd);
//    serverFd3  = socket(AF_INET,SOCK_STREAM,0);
//    printf("ser %d,ser2=%d,ser3=%d\n",serverFd,serverFd2,serverFd3);
	
	/* 
	 第一个参数：网络类型   ipv4 AF_INET，
						    ipv6 AF_INET6
							unix AF_UNIX
	 第二个参数：tcp SOCK_STREAM
	             UDP  SOCK_DGRAM
				 
	 第三个参数：通常填0
	 
 	*/
	if(serverFd<0){
		printf("Failed to create socket \n");
		return -1;
	}
	
	serverAdd.sin_family = AF_INET;    /* 网络类型*/
	serverAdd.sin_port = htons(atoi(argv[2]));  /* 端口注意字节序转换*/
	serverAdd.sin_addr.s_addr = inet_addr(argv[1]);   /* IP地址要转换成32位网络地址*/

	int reuse;// 设置地址重用，克服连接断开需要等待time_wait的时间。
	if ( setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, (void *)&reuse, sizeof(reuse)) < 0 ) {
		printf ("setsockopt SO_REUSEADDR failed \n");
		close (serverFd);
		return 0;
	};
	
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

    //创建epoll   epoll_create
    int fdCount;
    struct epoll_event event;
    struct epoll_event events[20];
    
    epfd = epoll_create(1); //创建epoll的句柄
    
    event.data.fd = serverFd;
    event.events = EPOLLIN | EPOLLET;
    
    //注册epoll事件 epoll_ctl
    epoll_ctl (epfd, EPOLL_CTL_ADD, serverFd, &event);

    while(1)
    {
        //等待事件到来，返回事件的数量
        fdCount=epoll_wait(epfd,events,20,-1);
        for(i=0;i<fdCount;++i)   //轮询等待到的事件
        {
            
            if(events[i].data.fd == serverFd)//判断是否是连接事件
            {
                clientFd = accept(events[i].data.fd,(struct sockaddr *)&clientAdd,&len);  
                event.data.fd=clientFd;
                event.events=EPOLLIN|EPOLLOUT|EPOLLET;
                epoll_ctl(epfd,EPOLL_CTL_ADD,clientFd,&event);
            }
            else if(events[i].events & EPOLLIN)  //判断是数据到来
            {
                memset(buff,0,100);
                ret = read(events[i].data.fd,buff,100); 
                if(ret <= 0){      //连接关闭，清除关注的事件描述符事件表    
                    epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,&event); 
                    close(events[i].data.fd);
                    continue;
                }else {
                    printf("epoll receve:%s\n",buff);
                }
            }
        }
    }





#if 0

    fd_set rdfs,tempfs;
    int maxfd,readycount;
    
    FD_ZERO(&rdfs);  //文件描述符集合清零
    FD_ZERO(&tempfs);      //文件描述符集合清零
    FD_SET(serverFd,&rdfs);  //添加要监听的文件描述符
    FD_SET(0,&rdfs);       //添加要监听的文件描述符

    maxfd = serverFd+1;   //设置最大的文件描述符+1

    while(1){

        //printf("before select rd=%d\n",rdfs);
        tempfs = rdfs;
        readycount = select(maxfd, &tempfs, NULL,NULL, NULL);   //等待连接或数据到来
        if(readycount<0){
             perror("failed to select:");
             return -1;
        
        }else if(readycount == 0){
           printf("time out\n"); 
           continue;

        }
        
        //printf("after select rd=%d\n",rdfs);
        
        for(i=0;i<maxfd;i++){                 //遍历所有的监听的文件描述符

              if(FD_ISSET(i,&tempfs)){         //检查监听的文件描述符是否有事件到来

                  if(i==0){   // 分类查看究竟是什么事件
                       
                      printf("keyboard\n");
                      
                  }else if(i==serverFd){  // 分类查看究竟是什么事件
                     clientFd = accept(i,(struct sockaddr *)&clientAdd,&len);  
                     printf("new client:ip %s,%d\n",inet_ntoa(clientAdd.sin_addr), ntohs(clientAdd.sin_port));
                     FD_SET(clientFd,&rdfs);   //新的连接要加到监听的文件描述符列表中
                     if(clientFd>maxfd-1){    //新连接可能改变最大文件描述符值
                        maxfd =clientFd+1; 
                     }
                     
                  }
                  else{  // 分类查看究竟是什么事件
                     memset(buff,0,100);
                     ret = read(i,buff,100); //读对应的准备好的socketFD
                     if(ret == 0){           //客户端关闭，清除对应的socketFD
                         FD_CLR(i,&rdfs);   
                         close(i);
                         continue;
                     }
                     printf("get client data:%s\n",buff);
                    
                  }

              }
     //       FD_ISSET(0,&rdfs);
     //       FD_ISSET(serverFd,&rdfs);


        }


    }    

#endif

#if 0
    
    
    

	
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

#endif
	
}
