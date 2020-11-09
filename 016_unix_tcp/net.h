#ifndef __NET_H__
#define __NET_H__

#include <stdio.h>
#include <arpa/inet.h> //inet_addr htons
#include <sys/types.h>
#include <sys/socket.h>  //socket bind listen accept connect
#include <netinet/in.h>  //sockaddr_in
#include <stdlib.h> //exit
#include <unistd.h> //close
#include <string.h> 
#include <strings.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/ip.h> /* superset of previous */
#include <linux/un.h>

#define errlog(errmsg) do{\
							perror(errmsg);\
							printf("%s -- %s -- %d\n", __FILE__, __func__, __LINE__);\
							exit(1);\
						 }while(0)

#define N 128

#endif    /* __NET_H__ */

