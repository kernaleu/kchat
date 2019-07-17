#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "config.h"
#include "server.h"
#include "command.h"
#include "filehandler.h"

#define ONLY 0
#define EXCEPT 1
#define EVERYONE 2

void remove_nl(char *str);
int resolve_nick(char *nick);
void server_send(int mode, int uid, const char *format, ...);
