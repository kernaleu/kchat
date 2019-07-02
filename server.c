#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>  
#include <signal.h>
#include "clients.h" // <netinet/in.h>
#include "config.h"

/* server socket descriptor */
int sockfd;

void err_exit(char *s)
{
    perror(s);
    exit(1);
}

void cleanup()
{
    send_all(" * Shutting down server\n");
    printf("exiting...\n");
    for (int i = 0; i < MAXCLI; i++)
        free(client[i]);
    close(sockfd);
    exit(0);
}

void main(int argc, char *argv[])
{
    port = PORT, buffsize = BUFFSIZE, maxcli = MAXCLI;

    int opt;

/* usage: ./server -p [port] -b [buffsize] -m [maxclient] */

    while ((opt = getopt(argc, argv, "p:b:m:")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'b':
                buffsize = atoi(optarg);
                if (buffsize < 20) {
                    printf("Buffer size must be at least 20\n");
                    buffsize = 20;
                }
                break;
            case 'm':
                maxcli = atoi(optarg);
                break;
            default:
                break;
        }
    }
    
    connected = 0;
    struct sockaddr_in serv_addr;
    
    /* create socket */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err_exit("socket");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    /* ignore SIGPIPE */
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, cleanup);

    if (bind(sockfd, (struct sockaddr *)&serv_addr , sizeof(serv_addr)) < 0)
        err_exit("bind");

	if (listen(sockfd, 10) < 0)
        err_exit("listen");
		
    printf("* Listening on port %d\n", port);
  
    init_clients();
    accept_clients();
}
