#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "client.h"
#include "server.h"
#include "colors.h"

int accept_clients()
{
    int connfd = 0;
    struct sockaddr_in cli_addr;
    
    uid = 0;
    /* accept clients */
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
            client[uid]->color = rand() % 12;
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
    char buf_in[BUFFSIZE-11];
    char buf_out[BUFFSIZE];
  
    /* get input from client */
    /* no more 100% cpu load \(^_^)/ */
    int read;
    while ((read = recv(client->connfd, buf_in, BUFFSIZE, 0))  > 0) {
            buf_in[read] = '\0';
            if(buf_in[0] != '\0') {
                sprintf(buf_out, "\033%s[%d]\033[0m: %s", palette[client->color], client->id, buf_in);
                send_all(client->id, buf_out);
            }
            memset(buf_in, 0, sizeof(buf_out));
    }
    
    sprintf(buf_out, "[%d] disconnected\n", client->id);
    send_all(client->id, buf_out);
    
    close(client->connfd);
    uid--;                      
    pthread_detach(pthread_self());
    return NULL;
}

void send_all(int sender_id, char *msg)
{
    for (int i = 0; i < uid; i++) {
        if (client[i]->id != sender_id)
            write(client[i]->connfd, msg, strlen(msg));
    }
    printf("%s", msg);
}
