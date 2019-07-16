#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "server.h"
#include "command.h"
#include "config.h"

void init_clients()
{
    client = malloc(sizeof(client_t *) * maxcli);
    for (int i = 0; i < maxcli; i++) {
        client[i] = malloc(sizeof(client_t));
        client[i]->mode = FREE;
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

        if (connected < maxcli) {
            connected++;
            for (int i = 0; i < maxcli; i++) {
                if (client[i]->mode == FREE) {
                    pthread_t tid;
                    /* Fill in the client struct */
                    client[i]->id = i;
                    client[i]->mode = BIN; /* Binary mode is default */
                    client[i]->color = rand() % 5 + 31;
                    client[i]->connfd = connfd;
                    client[i]->addr = cli_addr;
                    snprintf(client[i]->nick, 16, "guest-%d", i);
                    memset(client[i]->params, 0, PARAMS_SIZE);
                    pthread_create(&tid, NULL, handle_client, client[i]);
                    break;
                }
            }
        } else {
             printf("(srv) Max client number reached. Closing descriptor: %s.\n", inet_ntoa(cli_addr.sin_addr));
             close(connfd);
        }
    }
}

void *handle_client(void *arg)
{
    client_t *client = (client_t *)arg;

    ssize_t bytesread;
    char buf[bufsize];
    
    int fd;
    char filename[5] = {0};

    /* Get input from client */
    while ((bytesread = recv(client->connfd, buf, bufsize, 0)) > 0) {

        if (client->mode == BIN) {
            /* Do auth */
            if (buf[0] == '@') {
                puts("* Checking credentials..."); ///
                nick_set(client->id, buf);
                server_send(EVERYONE, 0, "\r\e[34m * %s joined. (connected: %d)\e[0m\n", client->nick, connected);
            }
            /* Download file */
            else if (buf[0] == '$') {
                puts("* We are in download mode..."); ///
                for (int i = 1; buf[i] != '\n'; i++) { // File id starts after '$' and ends before '\n'
                    if (i < 6) {
                        filename[i - 1] = buf[i]; // File id can't be larger than 5 characters (so we keep track of it) 
                    }
                    else {
                        server_send(ONLY, client->id, " * File id is too large!\n");
                        close(client->connfd);
                    }
                }
                fd = open(filename, O_RDONLY);
                if (fd) {
                    while ((bytesread = read(fd, buf, bufsize)) > 0) {
                        write(client->connfd, buf, bytesread);
                    }
                    close(fd);
                }
                close(client->connfd);
            }
            else {
                /* TODO: replace "test" with generated filename. */
                fd = open("test", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
                server_send(ONLY, client->id, "$test\n");

                write(fd, buf, bytesread);

                while ((bytesread = recv(client->connfd, buf, bufsize, 0)) > 0) {
                    write(fd, buf, bytesread);
                }

                close(fd);
                /* TODO: send out generated filename. */
                server_send(ONLY, client->id, "$test\n");
                close(client->connfd);
            }
        }

        if (bytesread < bufsize) {
            buf[bytesread + 1] = '\0';
        }
        if (strlen(buf) > 1) { /* Block empty messages */
            /* Handle commands */
            if (buf[0] == '/') {
                remove_nl(buf); 
                char *cmd = strtok(buf, " ");
                char *arg = strtok(NULL , " ");

                if (strcmp("/nick", cmd) == 0) {
                    cmd_nick(client->id, arg);
                } else if (strcmp("/list", buf) == 0) {
                    list_users(client->id);
                } else if (strcmp("/register", buf) == 0) {
                    nick_reg(client->id, arg);
                } else if (strcmp("/dm", cmd) == 0) {
                    direct_msg(client->id, arg);
                } else {
                    server_send(ONLY, client->id, "\r\e[34m * Unknown command.\n");
                }
            } else {
                /* Send message */
                server_send(EXCEPT, client->id, "\r\e[1;%dm%s\e[0m: %s", client->color, client->nick, buf);
            }
        }
    }

    client->mode = 0;
    connected--;
    server_send(EXCEPT, client->id, "\r\e[34m * %s left. (connected: %d)\e[0m\n", client->nick, connected);
    pthread_detach(pthread_self());
    return NULL;
}

/* 
 * Sending mode:
 * 0. Send only to uid
 * 1. Send to everyone except uid
 * 2. Send to everyone (ignores uid)
 */
void server_send(int mode, int uid, const char *format, ...)
{
    char buf[outbufsize];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, outbufsize, format, args);
    va_end(args);

    if (mode == 0) {
        write(client[uid]->connfd, buf, strlen(buf));
    } else {
        for (int i = 0; i < maxcli; i++) {
            if (client[i]->mode > BIN) {
                if (mode == 2 || i != uid) {
                    write(client[i]->connfd, buf, strlen(buf));
                }
            }
        }
    }
}

int nick_set(int uid, char *authstr)
{
    char *nick;
    char *pass;
    int auth_fd = open("auth.txt", O_RDONLY);
    char line[bufsize];
    int reg = 0, set = 0;

    authstr++;
    remove_nl(authstr);

    if ((nick = strtok(authstr, ":")) == NULL) {
        nick = authstr;
    }

    reg = is_registered(auth_fd, nick, line);

    if (reg) {
        char *fpass = strchr(line, ':') + 1;
        remove_nl(fpass);
        if ((pass = strtok(NULL, ":")) == NULL) {
            server_send(ONLY, uid, "\r\e[34m * This nick is registered. No password supplied.\e[0m\n");
            client[uid]->mode = GUEST;
            set = 0;
        } else {
            if (strcmp(fpass, pass) == 0) {
                strncpy(client[uid]->nick, nick, 16);
                client[uid]->mode = AUTH;
                set = 1;
            } else {
                server_send(ONLY, uid, "\r\e[34m * Wrong password.\e[0m\n");
                client[uid]->mode = GUEST;
                set = 0;
            }
        }
    } else {
        strncpy(client[uid]->nick, nick, 16);
        client[uid]->mode = GUEST;
        set = 1;
    }

    close(auth_fd);
    return set;
}
