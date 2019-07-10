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

int cmd_nick(int uid, char *nick)
{
    if (nick == NULL) {
        server_send(0, uid, "\r\e[34m * Usage: /nick nickname or nick:pass\e[0m\n");
        return 0;
    }
    
    char oldnick[16];
    strncpy(oldnick, client[uid]->nick, 16);
    
    if (nick_reg(uid, nick)) {
        server_send(2, 0, " \e[34m* %s is now known as %s.\e[0m\n", oldnick, client[uid]->nick);
    }
    return 1;
}

void list_users(int uid)
{
    for (int i = 0; i <= maxcli && client[i]->mode != 0; i++) {
        if (i % 5 == 0 && i != 0) 
            server_send(0, uid, "%c", '\n');
        server_send(0, uid, "%s\t ", client[i]->nick);
    }

    server_send(0, uid, "%c", '\n');
}
