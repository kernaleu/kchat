#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <stdlib.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>

#define PORT 1337 // default
#define BUFFSIZE 1024

char *palette[] = {"[0;31m", "[0;32m", "[0;33m", "[0;34m", "[0;35m", "[0;36m"};

typedef struct {
    int id;
    int connfd;
    struct sockaddr_in addr;
    int color;
} client_t;

void *handle_client(void *arg);

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
    while (1) {
        pthread_t tid;
        socklen_t client_len = sizeof(cli_addr);
        connfd = accept(sockfd, (struct sockaddr *)&cli_addr, &client_len);
       
        /* fill in the client struct */
        client_t *cli = malloc(sizeof(client_t));
        cli->addr = cli_addr;
        cli->connfd = connfd;
        cli->id = uid++;
        cli->color = rand() % 5;
        
        pthread_create(&tid, NULL, handle_client, cli);
    }    
    return 0;
}

void *handle_client(void *arg)
{
    client_t *client = (client_t *)arg;
    printf("Connection from %s\n", inet_ntoa(client->addr.sin_addr));
    char msg[BUFFSIZE] = {0};
    
   /* get input from client */
   /* have more than 4 clients disconnected and enjoy the 100% cpu load :) */ 
   while(1) {
        int read = recv(client->connfd, msg, BUFFSIZE, 0);
        if (read > 0) {
            msg[read] = '\0';
            printf("\033%s[%d]\033[0m: %s", palette[client->color], client->id, msg);
        }
    }
    return 0;
}
