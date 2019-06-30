#ifndef CLIENT_H
#define CLIENT_H

#define MAXCLI 2 // change this to allow more people

typedef struct {
    int id;
    char uname[32];
    int connfd;
    struct sockaddr_in addr;
    int color;
} client_t;

int uid;
client_t *client[MAXCLI];

int accept_clients(); // main loop
void *handle_client();
void send_all(int sender_id, char *msg); // broadcast a message to all clients except the sender

#endif
