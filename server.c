#include <stdio.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <signal.h>
#include "server.h"
#include "client.h"

#define PORT 1337

int main() 
{
    struct sockaddr_in serv_addr;
    
    /* create socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    /* ignore SIGPIPE */
    signal(SIGPIPE, SIG_IGN);
   
    bind(sockfd, (struct sockaddr *)&serv_addr , sizeof(serv_addr));

	listen(sockfd, 10);
		
    printf("Listening on port %d\n", PORT);
    
    /* accept clients */       
    accept_clients();

    return 0;
}
