#ifndef CLIENT_H
#define CLIENT_H

#define MAXCLI 10 // change this to allow more people

typedef struct {
    int id;
    char uname[32];
    int connfd;
    struct sockaddr_in addr;
    int color;
} client_t;

int connected;
client_t *client[MAXCLI];

void init_clients();
void accept_clients(); // main loop
void send_client(int id, char *msg); // send a reply to single client only
void send_all(int sender_id, char *msg); // broadcast a message to all clients except the sender
void motd(int id); // send message of the day
void *handle_client();

#endif
