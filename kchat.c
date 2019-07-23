#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>  
#include <signal.h>
#include "server.h"
#include "config.h"
#include "common.h"

/* Server socket descriptor */
int sockfd;

void err_exit(char *s)
{
    perror(s);
    exit(1);
}

void cleanup()
{
    server_send(EVERYONE, 0, "\r * Shutting down server\n");
    printf(" exiting...\n");
    for (int i = 0; i < MAXCLI; i++) {
        close(client[i]->connfd);
        free(client[i]);
    }
    free(client);
    close(sockfd);
    exit(0);
}

void main(int argc, char *argv[])
{
    /* Set default values if not specified in cli options */
    port = PORT, bufsize = BUFSIZE, maxcli = MAXCLI; 
    
    /* usage: ./server -p [port] -b [buffsize] -m [maxclient] */
    int opt;
    while ((opt = getopt(argc, argv, "p:b:m:")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'b':
                bufsize = atoi(optarg);
                if (bufsize < 20) {
                    printf("Buffer size must be at least 20\n");
                    bufsize = 20;
                }
                break;
            case 'm':
                maxcli = atoi(optarg);
        }
    }
    
    outbufsize = bufsize + 30; /* Give some more space for formating */
    connected = 0;
    struct sockaddr_in serv_addr;
    
    /* Create socket */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) err_exit("socket");

    int option = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    /* Ignore SIGPIPE */
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, cleanup);

    if (bind(sockfd, (struct sockaddr *)&serv_addr , sizeof(serv_addr)) < 0) err_exit("bind");
	if (listen(sockfd, 10) < 0) err_exit("listen");
		
    printf(" * Listening on port %d\n", port);
  
    init_clients();
    accept_clients();
}
