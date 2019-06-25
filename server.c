#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <stdlib.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include "client.h"

#define PORT 9999 
#define BUFFSIZE 1024
#define MAXCLI 2

client_t *client[MAXCLI];
int uid = 0;

void *handle_client(void *arg);
void send_all(int client_id, char *msg);

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
    
    /* accept client */       
    while (1) {
        socklen_t client_len = sizeof(cli_addr);
        connfd = accept(sockfd, (struct sockaddr *)&cli_addr, &client_len);
       
        if (uid+1 > MAXCLI) {
            printf("Max client number reached. Closing descriptor: %s.\n", inet_ntoa(cli_addr.sin_addr));
            close(connfd);
        } else {
            pthread_t tid;
            /* fill in the client struct */
            client[uid] = malloc(sizeof(client_t));
            client[uid]->addr = cli_addr;
            client[uid]->connfd = connfd;
            client[uid]->color = rand() % 6;
            client[uid]->id = uid;            
        
            pthread_create(&tid, NULL, handle_client, client[uid]);
            uid++;
        }
        sleep(1);
    }    
    return 0;
}

void *handle_client(void *arg)
{
    client_t *client = (client_t *)arg;
    printf("Connection from %s\n", inet_ntoa(client->addr.sin_addr));
    char buf_in[BUFFSIZE];
    char buf_out[BUFFSIZE];
    
   /* get input from client */
   /* have more than 4 clients disconnected and enjoy the 100% cpu load :) */ 
   while(1) {
        int read = recv(client->connfd, buf_in, BUFFSIZE, 0);
        if (read > 0) {
            buf_in[read] = '\0';
            printf("\033%s[%d]\033[0m: %s", palette[client->color], client->id, buf_in);
            sprintf(buf_out, "\033%s[%d]\033[0m: %s", palette[client->color], client->id, buf_in);
            send_all(client->id, buf_out);
        }
    }
    return 0;
}

void send_all(int sender_id, char *msg)
{
    for (int i = 0; i < uid; i++) {
        if (client[i]->id != sender_id)
            write(client[i]->connfd, msg, strlen(msg));
    }
}
