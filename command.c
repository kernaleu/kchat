#include <stdio.h>
#include <string.h>
#include "clients.h"
#include "command.h"

void remove_nl(char *arg)
{
    for (int i = 0; arg[i] != '\0'; i++) {
        if (arg[i] == '\n' || arg[i] == '\r')
            arg[i] = '\0';
    }                                            
}

int cmd_nick(int type, int uid, char *nick)
{
    char oldnick[16];
    strncpy(oldnick, client[uid]->nick, 16);
    strncpy(client[uid]->nick, nick, 16);
    if (type) server_send(2, 0, "\r\e[34m * %s is now known as %s.\e[0m\n", oldnick, client[uid]->nick);
    return 1;
}
