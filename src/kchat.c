#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../include/kchat.h"
#include "../include/commands.h"
#include "../include/str2argv.h"

static int bufinsize = BUF_IN_SIZE;
static int bufoutsize = BUF_OUT_SIZE;
static char *motd = MOTD;

int sockfd;
int maxclients = MAX_CLIENTS;
int connected = 0;

void quit() {
    puts("\r(serv) Shutting down...");
    server_send(EVERYONE, 0, "\r\e[34m * Server is shutting down...\e[0m\n");
    for (int id = 0; id < maxclients; id++)
        if (clients[id] != NULL)
            close(clients[id]->connfd);
    close(sockfd);
    exit(0);
}

int main(int argc, char *argv[]) {
    int conffd, connfd, id;
    fd_set descriptors;
    char buf[bufinsize + 1];

    clients = malloc(sizeof(client_t *) * maxclients);
    for (id = 0; id < maxclients; id++)
        clients[id] = NULL;

    struct sockaddr_in serv_addr, cli_addr;
    int addrlen = sizeof(cli_addr);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return 1;
    }

    int option = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(sockfd, 10) < 0) {
        perror("listen");
        return 1;
    }

    signal(SIGINT, quit);

    puts("(serv) Waiting for connections...");
    while (1) {
        FD_ZERO(&descriptors);
        FD_SET(sockfd, &descriptors);
        int maxfd = sockfd;
        /* Add all socket descriptors to the read list. */
        for (id = 0; id < maxclients; id++) {
            if (clients[id] != NULL) {
                FD_SET(clients[id]->connfd, &descriptors);
                /* Find highest file descriptor, needed for the select function. */
                if (clients[id]->connfd > maxfd)
                    maxfd = clients[id]->connfd;
            }
        }

        select(maxfd + 1 ,&descriptors, NULL, NULL, NULL);
        /* Incoming connection on the primary socket. (new client) */
        if (FD_ISSET(sockfd, &descriptors)) {
            if ((connfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t*)&addrlen)) < 0) {
                perror("accept");
                exit(1);
            }
            printf("(serv) New connection, sockfd: %d, ipaddr: %s, port: %d\n", connfd, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
            for (id = 0; id < maxclients; id++) {
                /* If position is empty */
                if (clients[id] == NULL) {
                    clients[id] = malloc(sizeof(client_t));
                    clients[id]->connfd = connfd;
                    clients[id]->color = rand() % 5 + 31;
                    snprintf(clients[id]->nick, 16, "guest-%d", id);
                    connected++;
                    server_send(EXCEPT, id, "\r\e[34m * %s joined. (connected: %d)\e[0m\n", clients[id]->nick, connected);
                    server_send(ONLY, id, "%s\n", motd);
                    break;
                }
            }
        }
        /* IO operations on other sockets. */
        for (id = 0; id < maxclients; id++) {
            if (clients[id] != NULL) {
                if (FD_ISSET(clients[id]->connfd, &descriptors)) {
                    ssize_t bytesread;
                    if ((bytesread = read(clients[id]->connfd, buf, bufinsize)) > 0) {
                        buf[bytesread] = '\0';
                        remove_nl(buf);

                        /* Skip empty messages. */
                        if (strlen(buf) == 0)
                            continue;
                        /* Handle commands. */
                        if (buf[0] == '/')
                            command_handler(id, buf);
                        /* Send message. */
                        else
                            server_send(EXCEPT, id, "\r\e[1;%dm%s\e[0m: %s\n", clients[id]->color, clients[id]->nick, buf);
                    }
                    /* Client disconnected. */
                    else {
                        close(clients[id]->connfd);
                        connected--;
                        server_send(EXCEPT, id, "\r\e[34m * %s left. (connected: %d)\e[0m\n", clients[id]->nick, connected);
                        free(clients[id]);
                        clients[id] = NULL;
                    }
                }
            }
        }
    }
    return 0;
}

/*
 * Sending mode:
 * 0. Send only to id
 * 1. Send to everyone except id
 * 2. Send to everyone (ignores id)
 */
void server_send(int mode, int id, const char *format, ...) {
    char buf[bufoutsize];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, bufoutsize, format, args);
    va_end(args);

    if (mode == ONLY)
        write(clients[id]->connfd, buf, strlen(buf));
    else
        for (int i = 0; i < maxclients; i++)
            if ((mode == EVERYONE || i != id) && clients[i] != NULL)
                write(clients[i]->connfd, buf, strlen(buf));
}

int resolve_nick(char *nick) {
    for (int id = 0; id < maxclients; id++)
        if (clients[id] != NULL && strncmp(clients[id]->nick, nick, 16) == 0)
            return id;
    /* Queried nick didn't match any. */
    return -1;
}

void remove_nl(char *str) {
    for (int i = 0; str[i] != '\0'; i++)
        if (str[i] == '\n' || str[i] == '\r') {
            str[i] = '\0';
            break;
        }
}

int change_nick(int id, char *str) {
    if (resolve_nick(str) != -1)
        return 0;
    strncpy(clients[id]->nick, str, 16);
    clients[id]->nick[16]= '\0';
    return 1;
}

void command_handler(int id, char *str) {
    const char *errmsg;
    char **argv;
    int argc;

    if (str2argv(str, &argc, &argv, &errmsg) != 0) {
        server_send(ONLY, id, "\r\e[31m * %s!\e[0m\n", errmsg);
        return;
    }

    for (int i = 0; i < argc; i++)
        printf("(serv) argv[%d] = \"%s\"\n", i, argv[i]);

    if (strcmp("/nick", argv[0]) == 0)
        cmd_nick(id, argc, argv);
    else if (strcmp("/dm", argv[0]) == 0)
        cmd_dm(id, argc, argv);
    else if (strcmp("/users", argv[0]) == 0)
        cmd_users(id);
    else if (strcmp("/restart", argv[0]) == 0)
        quit();
    else if (strcmp("/register", argv[0]) == 0)
        cmd_register(id, argc, argv);
    else if (strcmp("/login", argv[0]) == 0)
        cmd_login(id, argc, argv);
    else
        server_send(ONLY, id, "\r\e[31m * Unknown command!\e[0m\n");

    argv_free(&argc, &argv);
}
