#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "server.h"
#include "command.h"
#include "config.h"

void init_clients()
{
    client = malloc(sizeof(client_t *) * maxcli);
    for (int i = 0; i < maxcli; i++) {
        client[i] = malloc(sizeof(client_t));
        client[i]->mode = 0;
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
                if (client[i]->mode == 0) {
                    pthread_t tid;
                    /* Fill in the client struct */
                    client[i]->id = i;
                    client[i]->mode = 1; /* Binary mode is default */
                    /* TODO: Move this part out of here (Because of binary mode) */
                    client[i]->color = rand() % 5 + 31;
                    client[i]->connfd = connfd;
                    client[i]->addr = cli_addr;
                    snprintf(client[i]->nick, 16, "guest-%d", i);
                    memset(client[i]->params, 0, PARAMS_SIZE);
                    /* --------------------------------------------------------- */
                    pthread_create(&tid, NULL, handle_client, client[i]);
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

    int netread, auth = 1, filemode = 0;
    char buf[bufsize], filename[5];

    FILE *f;
    
    char authstr[bufsize];
    read(client->connfd, authstr, bufsize-1);
    nick_set(client->id, authstr);

    server_send(2, 0, "\r\e[34m * %s joined. (connected: %d)\e[0m\n", client->nick, connected);
    
    /* Get input from client */
    while ((netread = recv(client->connfd, buf, bufsize-1, 0)) > 0) {
        buf[netread] = '\0';

        /* Check if we are still in binary mode */
        /* if (client->mode == 1) {
            if (buf[0] == '$') { // If 1st char is '$' it has to be binary mode 
                printf("dbg: Yes 1st char is indeed '$'.\n");
                if (buf[1] == '\n') { // This is upload mode 
                    printf("dbg: Yes 2nd char is indeed '\\n' this is upload mode.\n");
                    if (filemode == 0) {
                        printf("dbg: Opening file for write.\n");
                        snprintf(filename, 5, "test"); // TODO: Generate filename here 
                        f = fopen(filename, "a");
                        filemode = 1;
                    }
                } else { // This is download mode 
                    printf("dbg: This is download mode.\n");
                    for (int i = 1; buf[i] != '\n'; i++) { // File id starts after '$' and ends before '\n' 
                        if (i < 6) {
                            filename[i - 1] = buf[i]; // File id can't be larger than 5 characters (so we keep track of it) 
                            printf("dbg: %d %s\n", i, filename);
                        } else {
                            printf("dbg: %d %s Smh wrong with provided name going back to guest mode.\n", i, filename);
                            client->mode = 2;
                            break;
                        }
                    }
                    // Open file 
                    printf("dbg: Opening file (%s) for read.\n", filename);
                    f = fopen(filename, "a");
                    int fileread, bufsize2 = bufsize;

                    // Get file size 
                    fseek(f, 0, SEEK_END);
                    long fs = ftell(f);
                    fseek(f, 0, SEEK_SET);

                    printf("dbg: Opened file size is %dl.\n", fs);

                    // Send file 
                    while ((fileread = read(f, &buf, bufsize)) > 0) {
                        if (fs > bufsize) fs -= bufsize;
                        else bufsize2 = fs;
                        write(client->connfd, buf, bufsize2);
                    }

                    // Close file
                    fclose(f);
                } 
            } else { // If 1st character is not '$' go to guest mode 
                client->mode = 2;
            }
        } */

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
                } else {
                    server_send(0, client->id, "%s\n", "unknown command");
                }
            } else {
                /* Send message */
                server_send(1, client->id, "\r\e[1;%dm%s\e[0m: %s", client->color, client->nick, buf);
            }
        }
    }

    client->mode = 0;
    connected--;
    server_send(2, 0, "\r\e[34m * %s left. (connected: %d)\e[0m\n", client->nick, connected);
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
            if (client[i]->mode > 1) {
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
    FILE *auth_fp = fopen("auth.txt", "ra");
    char line[1024];
    int registered = 0, set = 0;

    remove_nl(authstr);

    if ((nick = strtok(authstr, ":")) == NULL) {
        nick = authstr;
    }
    while (fgets(line, bufsize, auth_fp) != NULL) {
        /* the nick is registered and is in the auth file.*/           
        if (strstr(line, nick)) {
            registered = 1;
            break;
        }
    }
        
    if (registered) {
        char *fpass = strchr(line, ':') + 1;
        remove_nl(fpass);
        if ((pass = strtok(NULL, ":")) == NULL) {
            server_send(0, uid, "\r\e[34m * This nick is registered. No password supplied.\e[0m\n");
            client[uid]->mode = 2;
            set = 0;
        } else {
            if (strcmp(fpass, pass) == 0) {
                strcpy(client[uid]->nick, nick);
                client[uid]->mode = 3;
                set = 1;
            } else {
                server_send(0, uid, "\r\e[34m * Wrong password.\e[0m\n");
                client[uid]->mode = 2;
                set = 0;
            }
        }
    } else {
        strcpy(client[uid]->nick, nick);
        client[uid]->mode = 2;
        set = 1;
    }       
    fclose(auth_fp);
    return set;
}
