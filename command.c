#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "server.h"
#include "command.h"

void remove_nl(char *arg)
{
    for (int i = 0; arg[i] != '\0'; i++) {
        if (arg[i] == '\n' || arg[i] == '\r')
            arg[i] = '\0';
    }                                            
}

void direct_msg(int uid, char *arg)
{
    // Parse arg into nick and the actual message.
}

int resolve_nick(char *nick)
{
    for (int i = 0; i < maxcli; i++) {
        if (strcmp(client[i]->nick, nick) == 0) {
            return i;
        }
    }
    /* Queried nick didn't match any. */
    return -1;
}

/* Check if the nickname is in the auth file */
int is_registered(int fd, char *nick, char *line) 
{
    /* Max length allowed for nick is 16, + 1 for : */
    char str[17];
    while (read(fd, line, bufsize) > 0) {
        if (strstr(line, strcat(strncpy(str, nick, 16), ":"))) {
            return 1;
        }
    }
    return 0;
}

void cmd_nick(int uid, char *nick)
{
    if (nick == NULL) {
        server_send(ONLY, uid, "\r\e[34m * Usage: /nick nickname or nick:pass\e[0m\n");
    } else {
        char oldnick[16];
        strncpy(oldnick, client[uid]->nick, 16);
    
        if (nick_set(uid, nick)) {
            server_send(EVERYONE, 0, " \e[34m* %s is now known as %s.\e[0m\n", oldnick, client[uid]->nick);
        }
    }
}

void nick_reg(int uid, char *authstr)
{;}

void list_users(int uid)
{
    for (int i = 0; i <= maxcli && client[i]->mode != 0; i++) {
        if (i % 5 == 0 && i != 0) 
            server_send(ONLY, uid, "%c", '\n');
        server_send(ONLY, uid, "%s\t ", client[i]->nick);
    }

    server_send(ONLY, uid, "%c", '\n');
}


