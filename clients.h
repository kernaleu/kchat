#ifndef CLIENTS_H
#define CLIENTS_H

/* netinet included here because we have a struct member of type sockaddr_in.
   if we hadn't included this here then we would have to include this in
   every other .c file this header is used in even though it's completely
   unrelated to the .c file's contents. */
   
#include <netinet/in.h> 

typedef struct {
    int id;
    char nick[16];
    int connfd;
    struct sockaddr_in addr;
    int color;
} client_t;

client_t **client;

int port;
int buffsize;
int maxcli;
extern int sockfd;

int connected;

void init_clients();
void accept_clients(); /* main loop */
void server_send(int mode, int uid, const char *format, ...); /* send messages */
void *handle_client();

#endif
