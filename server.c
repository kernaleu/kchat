#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <signal.h>
#include "server.h"
#include "client.h"

#define PORT 9999

void err_exit(char *s)
{
    perror(s);
    exit(1);
}

void cleanup()
{
    printf(" exiting...\n");
    for (int i = 0; i < MAXCLI; i++)
        free(client[i]);

    exit(0);
}

int main() 
{
    struct sockaddr_in serv_addr;
    
    /* create socket */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err_exit("socket");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    /* ignore SIGPIPE */
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, cleanup);

    if (bind(sockfd, (struct sockaddr *)&serv_addr , sizeof(serv_addr)) < 0)
        err_exit("bind");

	if (listen(sockfd, 10) < 0)
        err_exit("listen");
		
    printf("Listening on port %d\n", PORT);
    
    /* accept clients */       
    accept_clients();

    close(sockfd);

    return 0;
}
