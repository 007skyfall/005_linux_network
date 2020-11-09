#ifndef __NET_H__
#define __NET_H__

#include <stdio.h>
#include <arpa/inet.h> //inet_addr htons
#include <sys/types.h>
#include <sys/socket.h>  //socket bind listen accept connect
#include <netinet/in.h>  //sockaddr_in
#include <stdlib.h> //exit
#include <unistd.h> //close

#define errlog(errmsg) do{\
							perror(errmsg);\
							printf("%s -- %s -- %d\n", __FILE__, __func__, __LINE__);\
							exit(1);\
						 }while(0)

#endif    /* __NET_H__ */