typedef struct {
    int id;
    int connfd;
    struct sockaddr_in addr;
    int color;
} client_t;

char *palette[] = {
    "[0;31m", 
    "[0;32m", 
    "[0;33m", 
    "[0;34m", 
    "[0;35m", 
    "[0;36m"
};
