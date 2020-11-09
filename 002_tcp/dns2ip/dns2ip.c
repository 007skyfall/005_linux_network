#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int dns2ip(const char *domain, char * ip, int len)
{
    char **pptr;  
    struct hostent *hptr;  
#if 0

/* Description of data base entry for a single host.  */
struct hostent
{
  char *h_name;                 /* Official name of host.  */
  char **h_aliases;             /* Alias list.  */
  int h_addrtype;               /* Host address type.  */
  int h_length;                 /* Length of address.  */
  char **h_addr_list;           /* List of addresses from name server.  */
#ifdef __USE_MISC
# define        h_addr  h_addr_list[0] /* Address, for backward compatibility.*/
#endif
};

#endif
    
    if((hptr = gethostbyname(domain)) == 0)                                                          
    {                                                                                                
        printf(" gethostbyname error for host:%s\n", domain);                                           
        return -1;
    }
    else
    {
//        printf("official hostname:%s\n", hptr->h_name);                                                   
        for(pptr = hptr->h_aliases; *pptr != 0; pptr++)                                               
        {
            printf(" alias:%s\n", *pptr);                                                                 
        }
        switch(hptr->h_addrtype)                                                                         
        {                                                                                                
            case AF_INET:                                                                                
            case AF_INET6:                                                                               
            pptr=hptr->h_addr_list;                                                                  
            for(; *pptr!=0; pptr++)
            {    
                /*  const char *inet_ntop(int af, const void *src,
                             char *dst, socklen_t size); */
                inet_ntop(hptr->h_addrtype, *pptr, ip, len);
            }               
            break; 
            
            default:                                                                                     
            printf("unknown address type\n");
            break;                                                                                       
        }       

    }
    
    return 0;
}

int main(int argc, const char *argv[])
{
    char ip[32] = { 0 };
    
	if(argc != 2)
    {
		fprintf(stderr, "Usage: %s <nds_name>\n", argv[0]);
		return -1;
	}

    memset(ip, 0, sizeof(ip));
	if(dns2ip(argv[1], ip, sizeof(ip)/sizeof(char)) < 0)
	{
        printf("dns2ip error!\n");
        return -2;
    }
    printf(" address:%s\n", ip);                        
    return 0;
}

