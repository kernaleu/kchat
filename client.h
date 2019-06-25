#ifndef CLIENT_H
#define CLIENT_H

typedef struct {
    int id;
    int connfd;
    struct sockaddr_in addr;
    int color;
} client_t;

int accept_clients(); // main loop
void *handle_client();
void send_all(int sender_id, char *msg); // broadcast a message to all clients except the sender

#endif
