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
        
        printf("(srv): connection attempt from: %s\n", inet_ntoa(cli_addr.sin_addr));

        if (connected <= MAXCLI) {
            connected++;
            for (int uid = 0; uid < MAXCLI; uid++) {
                if (client[uid]->connfd == 0) {
                    pthread_t tid;
                    /* fill in the client struct */
                    client[uid]->addr = cli_addr;
                    client[uid]->connfd = connfd;
                    client[uid]->color = rand() % 5 + 31;
                    client[uid]->id = uid;
                    //sprintf(client[uid]->nick, "%d", uid);
                    pthread_create(&tid, NULL, handle_client, client[uid]);
                    break;
                }
            }
        } else {
             printf("(srv) Max client number reached. Closing descriptor: %s.\n", inet_ntoa(cli_addr.sin_addr));
             close(connfd);
        }
        sleep(1);
    }
}

void *handle_client(void *arg)
{
    client_t *client = (client_t *)arg;

    int read, join = 0;
    char buf_in[BUFFSIZE];
    char buf_out[BUFFSIZE+13];

    /* Send motd */
    sprintf(buf_out, " \e[34m* \e[35m%s\n\e[34m * \e[35mPlease enter your nick.\e[0m\n", MOTD);
    send_client(client->id, buf_out);

    /* Get input from client */
    while ((read = recv(client->connfd, buf_in, BUFFSIZE-1, 0)) > 0) {
        buf_in[read] = '\0';

        for (int i = 0; i < strlen(buf_in); i++) {
            if (buf_in[i] < ' ' || buf_in[i] > '~') buf_in[i] = ' ';

        }

        if (strlen(buf_in) > 1) {
            if (!join) {
                if (cmd_nick(0, client->id, buf_in)) {
                    join = 1;
                    sprintf(buf_out, " \e[34m* %s joined. (connected: %d)\e[0m\n", client->nick, connected);
                    send_all(buf_out);
                }
            } else {
                if (buf_in[0] == '/') {
                    char *cmd = strtok(buf_in, " ");
                    if (strcmp("/nick", cmd) == 0) {
                        char *arg = strtok(NULL , " ");
                        cmd_nick(1, client->id, arg);
                    }
                } else {
                    sprintf(buf_out, "\e[1;%dm%s\e[0m: %s\n", client->color, client->nick, buf_in);
                    send_msg(client->id, buf_out);
                }
            }
        }
        memset(buf_in, 0, sizeof(buf_in));
    }

    client->connfd = 0;
    connected--;
    sprintf(buf_out, " \e[34m* %s left. (connected: %d)\e[0m\n", client->nick, connected);
    send_all(buf_out);

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

void send_msg(int sender_uid, char *msg)
{
    for (int uid = 0; uid < MAXCLI; uid++) {
        if (client[uid]->connfd != 0 && uid != sender_uid) {
            write(client[uid]->connfd, msg, strlen(msg));
        }
    }
    printf("%s", msg);
}

void send_client(int uid, char *msg)
{
    write(client[uid]->connfd, msg, strlen(msg));
    //printf("\e[1;31mserver (%s): \e[0m: %s", client[uid]->nick, msg);
}
