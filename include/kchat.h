#ifndef KCHAT_H
#define KCHAT_H

#include "../config.h"
#define BUF_OUT_SIZE BUF_IN_SIZE+14

extern int sockfd;
extern int maxclients;
extern int connected;

typedef struct {
    int connfd;
    int color;
    char nick[17];
} client_t;

client_t **clients;

#define ONLY 0
#define EXCEPT 1
#define EVERYONE 2
void server_send(int mode, int id, const char *format, ...);
int resolve_nick(char *nick);
void remove_nl(char *str);
int change_nick(int id, char *str);
void command_handler(int id, char *str);

#endif
