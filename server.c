#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <stdlib.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <signal.h>

#define PORT 1337 // default
#define BUFFSIZE 1024

typedef struct {
    int id;
    int connfd;
    struct sockaddr_in addr;
} client_t;

int handle_client(client_t *client);

int main() 
{
    int sockfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;

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
    
    int uid = 0;

    /* accept client */       
    socklen_t client_len = sizeof(cli_addr);
    connfd = accept(sockfd, (struct sockaddr *)&cli_addr, &client_len);
        
    /* fill in the client struct */
    client_t *cli = malloc(sizeof(client_t));
    cli->addr = cli_addr;
    cli->connfd = connfd;
    cli->id = uid++;
        
    handle_client(cli);
    
    return 0;
}

int handle_client(client_t *client)
{
    printf("Connection from %s\n", inet_ntoa(client->addr.sin_addr));
    char msg[BUFFSIZE] = {0};
    
    /* get input from client */
    while(1) {
        int read = recv(client->connfd, msg, BUFFSIZE, 0);
        if (read > 0) {
            msg[read] = '\0';
            printf("[%d]: %s",client->id, msg);
        }
    }
    return 0;
}
