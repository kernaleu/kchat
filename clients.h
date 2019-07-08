#ifndef CLIENTS_H
#define CLIENTS_H

#include "config.h"

/* netinet included here because we have a struct member of type sockaddr_in.
   if we hadn't included this here then we would have to include this in
   every other .c file this header is used in even though it's completely
   unrelated to the .c file's contents. */

#include <netinet/in.h> 

/*
 * mode:
 *   0. Free (no user is connected)
 *   1. Binary transfer mode (used for sending/receiving files)
 *   2. Unregistered (user connected using guest account)
 *   3. Registered (user connected and entered auth hash)
 *
 * color:
 *   31. Red
 *   32. Green
 *   33. Yellow
 *   34. Blue
 *   35. Magenta
 *   36. Cyan
 *
 * params:
 *   [0]. beep:
 *     0. Disabled for all
 *     1. Enabled for all
 *     2. Enabled just for registered users
*/

typedef struct {
    int id;
    int mode;
    int color;
    char nick[16];
    int params[PARAMS_SIZE];
    int connfd;
    struct sockaddr_in addr;
} client_t;

client_t **client;

int port;
int bufsize;
int maxcli;
int outbufsize;

extern int sockfd;

unsigned int connected;

void init_clients();
void accept_clients(); /* Main loop */
void server_send(int mode, int uid, const char *format, ...); /* Send messages */
void *handle_client();

#endif
