#include <stdio.h>
#include <string.h>
#include "client.h"
#include "command.h"

void remove_nl(char *arg)
{
    for (int i = 0; arg[i] != '\0'; i++) {
        if (arg[i] == '\n' || arg[i] == '\r')
            arg[i] = '\0';
    }                                            
}

/*
int validate_message(char *msg)
{
    for (int i = 0; msg[i] != '\0'; i++) {
        if (arg[i] >)
    }
}
*/

int cmd_nick(int type, int uid, char *nick)
{
    char oldnick[16];
    char tmp[62];
    //remove_nl(nick);
    strncpy(oldnick, client[uid]->nick, 16);
    strncpy(client[uid]->nick, nick, 16);
    if (type) {
        sprintf(tmp, " \e[34m* %s is now known as %s.\e[0m\n", oldnick, client[uid]->nick);
        send_all(tmp);
    }
    return 1;
}
