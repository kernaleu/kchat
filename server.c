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
#include "common.h"
#include "filehandler.h"
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

    bytesread = recv(client->connfd, buf, bufsize, 0);
    switch (buf[0]) {
        case '@':
            client->mode = GUEST;
            remove_nl(buf);
            user_login(client->id, buf + 1);
            motd_send();
            server_send(EVERYONE, 0, "\r\e[34m * %s joined. (connected: %d)\e[0m\n", client->nick, connected);
            break;
        case '$':
            file_download(client->connfd, client->id, buf);
            break;
        default:
            file_upload(client->connfd, client->id, buf, bytesread);
    }

    /* Get input from client */
    while ((bytesread = recv(client->connfd, buf, bufsize, 0)) > 0) {
        if (bytesread < bufsize) {
            buf[bytesread + 1] = '\0';
        }
        remove_nl(buf);

        if (strlen(buf) > 0) { /* Block empty messages */
            /* Handle commands */
            if (buf[0] == '/') {
                /* TODO: Make own function to split cmd and args */
                char *cmd = strtok(buf, " ");
                char *arg = strtok(NULL, " ");
                if (strcmp("/login", cmd) == 0) {
                    cmd_login(client->id, arg);
                } else if (strcmp("/list", buf) == 0) {
                    cmd_list(client->id);
                } else if (strcmp("/register", buf) == 0) {
                    cmd_register(client->id, arg);
                } else if (strcmp("/dm", cmd) == 0) {
                    cmd_dm(client->id, arg);
                } else if (strcmp("/motd", cmd) == 0) {
                    motd_set(client->nick, arg);
                    motd_send();
                } else {
                    server_send(ONLY, client->id, "\r\e[34m * Unknown command.\e[0m\n");
                }
            } else {
                /* Send message */
                server_send(EXCEPT, client->id, "\r\e[1;%dm%s\e[0m: %s\n", client->color, client->nick, buf);
            }
        }
    }

    client->mode = 0;
    connected--;
    server_send(EXCEPT, client->id, "\r\e[34m * %s left. (connected: %d)\e[0m\n", client->nick, connected);
    pthread_detach(pthread_self());
    return NULL;
}

int user_login(int uid, char *str)
{
    char *nick, *pass;
    char fpass[256];
    int set = 0;

    nick = strtok(str, ":");
    pass = strtok(NULL, ":");

    printf(" * nick: \"%s\"\n", nick); ///
    printf(" * pass: \"%s\"\n", pass); ///

    if (resolve_nick(nick) >= 0) {
        server_send(ONLY, uid, "\r\e[34m * User with this nick is already logged in.\e[0m\n");
    } else if (find_pass(nick, fpass)) {
        if (pass != NULL) {
            if (strcmp(fpass, pass) == 0) {
                /* Correct pass, set the nick and the correct mode. */
                client[uid]->mode = AUTH;
                set = 1;
            } else {
                /* Incorrect pass, complain. */
                server_send(ONLY, uid, "\r\e[34m * Wrong password.\e[0m\n");
            }
        } else {
            /* Nick is registered and no password supplied, complain. */
            server_send(ONLY, uid, "\r\e[34m * This nick is registered. No password supplied.\e[0m\n");
        }
    } else {
        /* Nick is not registered and not in use. 
         * Set nick and the correct mode. */
        client[uid]->mode = GUEST;
        set = 1;
    }
    if (set) {
        strncpy(client[uid]->nick, nick, 16);
    }
    return set;
}

/* If password was found returns 1.
 * If password was not found returns 0.
 * If error returns -1. */
int find_pass(char *nick, char *pass)
{
    int r = 0;
    size_t linesize = 274; /* nick(16) + ':' + pass_sha256(256) + '\n' = 274 */
    char *line = NULL, *p;
    FILE *fp = fopen("auth.txt", "r");

    if (fp) {
        while (getline(&line, &linesize, fp) != -1) {
            p = strchr(line, ':');
            if (strlen(nick) == p - line && strncmp(nick, line, p - line) == 0) {
                remove_nl(line);
                strncpy(pass, p + 1, 256);
                r = 1;
                break;
            }
        }
        fclose(fp);
    } else {
        r = -1;
    }
    free(line);
    return r;
}
