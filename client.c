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

void init_clients()
{
    for (int i = 0; i < MAXCLI; i++) {
        client[i] = malloc(sizeof(client_t));
        client[i]->connfd = 0;
    }
}

void accept_clients()
{
    int connfd;
    struct sockaddr_in cli_addr;
    
    /* accept clients */
    while (1) {
        socklen_t client_len = sizeof(cli_addr);
        connfd = accept(sockfd, (struct sockaddr *)&cli_addr, &client_len);

        printf("[DBG] connected: %d\n", connected);

        if (connected <= MAXCLI) {
            connected++;
            printf("[DBG] still are free places\n");
            for (int uid = 0; uid < MAXCLI; uid++) {
                if (client[uid]->connfd == 0) {
                    pthread_t tid;
                    /* fill in the client struct */
                    client[uid]->addr = cli_addr;
                    client[uid]->connfd = connfd;
                    client[uid]->color = rand() % 12;
                    client[uid]->id = uid;
                    sprintf(client[uid]->uname, "%d", uid);
                    pthread_create(&tid, NULL, handle_client, client[uid]);
                    break;
                }
            }
        } else {
             printf("Max client number reached. Closing descriptor: %s.\n", inet_ntoa(cli_addr.sin_addr));
             close(connfd);
        }
        sleep(1);
    }
}

void *handle_client(void *arg)
{
    client_t *client = (client_t *)arg;
    printf("Connection from %s\n", inet_ntoa(client->addr.sin_addr));
    char buf_in[BUFFSIZE-11];
    char buf_out[BUFFSIZE];
    
    motd(client->id);

    /* get input from client */
    int read;
    while ((read = recv(client->connfd, buf_in, BUFFSIZE, 0)) > 0) {
            buf_in[read] = '\0';
            if (strlen(buf_in) > 1) {
                if (buf_in[0] == '/') {
                    // handle commands
                    ;
                } else {
                    sprintf(buf_out, "\033%s[%s]\033[0m: %s", palette[client->color], client->uname, buf_in);
                    send_all(client->id, buf_out);
                }
            }
            memset(buf_in, 0, sizeof(buf_in));
    }

    sprintf(buf_out, "[%d] disconnected\n", client->id);
    send_all(client->id, buf_out);
    
    client->connfd = 0;
    connected--;
    pthread_detach(pthread_self());
    return NULL;
}

void send_all(int sender_id, char *msg)
{
    for (int uid = 0; uid < MAXCLI; uid++) {
        if (client[uid]->id != sender_id && client[uid]->connfd != 0)
            write(client[uid]->connfd, msg, strlen(msg));
    }
    printf("%s", msg);
}

void send_client(int id, char *msg)
{
    write(client[id]->connfd, msg, strlen(msg));
    printf("[DBG] sent to client [%s]:%s", client[id]->uname, msg);
}

void motd(int id)
{
    char tmp[BUFFSIZE];
    sprintf(tmp, "%s || connected: %d\n", MOTD, connected);
    send_client(id, tmp);
}
