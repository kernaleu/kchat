#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "client.h"
#include "server.h"
#include "command.h"
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
        
        printf("[DBG] connection attempt from: %s\n", inet_ntoa(cli_addr.sin_addr));
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
                    printf("[DBG] accept\n");
                    break;
                }
            }
        } else {
             printf("[DBG] Max client number reached. Closing descriptor: %s.\n", inet_ntoa(cli_addr.sin_addr));
             close(connfd);
        }
        sleep(1);
    }
}

void *handle_client(void *arg)
{
    client_t *client = (client_t *)arg;

    char buf_in[BUFFSIZE-11];
    char buf_out[BUFFSIZE];

    motd(client->id);

    sprintf(buf_out, "** Joined: %s\n", client->uname);
    send_all(buf_out);

    /* get input from client */
    int read;
    while ((read = recv(client->connfd, buf_in, BUFFSIZE, 0)) > 0) {
            buf_in[read] = '\0';
            if (strlen(buf_in) > 1) {
                if (buf_in[0] == '/') {
                    char *cmd = strtok(buf_in, " ");
                    
                    if (strcmp("/name", cmd) == 0) {
                        char *arg = strtok(NULL , " ");
                        change_name(arg, client->id);
                    }
                } else {
                    sprintf(buf_out, "\033%s[%s]\033[0m: %s", palette[client->color], client->uname, buf_in);
                    send_msg(client->id, buf_out);
                }
            
            }
        memset(buf_in, 0, sizeof(buf_in));
    }

    sprintf(buf_out, "** Disconnected: %s\n", client->uname);
    send_all(buf_out);

    client->connfd = 0;
    connected--;
    pthread_detach(pthread_self());
    return NULL;
}

void send_all(char *msg)
{
    for (int uid = 0; uid < MAXCLI; uid++) {
        if (client[uid]->connfd != 0)
            write(client[uid]->connfd, msg, strlen(msg));
    }
    printf("%s", msg);
}

void send_msg(int id, char *msg)
{
    for (int uid = 0; uid < MAXCLI; uid++) {
        if (client[uid]->id != id && client[uid]->connfd != 0)
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
