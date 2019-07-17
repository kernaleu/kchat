#ifndef SERVER_H
#define SERVER_H

#include "common.h"

#define FREE 0
#define BIN 1
#define GUEST 2
#define AUTH 3

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
int user_login(int uid, char *str);

void *handle_client();

#endif
