#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "server.h"
#include "command.h"
#include "common.h"

void cmd_dm(int uid, char *arg)
{
    /* Usage: /dm nick msg */
    char *nick = strtok(arg, " ");
    char *msg = strtok(NULL, " ");
    int to_id = resolve_nick(arg);
    server_send(ONLY, to_id, "\r[DM] \e[1;%dm%s\e[0m: %s", client[uid]->color, client[uid]->nick, msg);
}

void cmd_login(int uid, char *arg)
{
    if (arg == NULL) {
        server_send(ONLY, uid, "\r\e[34m * Usage: /login nick or nick:pass\e[0m\n");
    } else {
        char oldnick[16];
        strncpy(oldnick, client[uid]->nick, 16);
    
        if (user_login(uid, arg)) {
            server_send(EVERYONE, 0, " \e[34m* %s is now known as %s.\e[0m\n", oldnick, client[uid]->nick);
        }
    }
}

void cmd_register(int uid, char *arg)
{;}

void cmd_motd(int uid, char *arg)
{
    strcpy(motd->msg, arg);
    strcpy(motd->nick, client[uid]->nick);
    server_send(EVERYONE, 0, "\e[34m * %s | set by %s\e[0m\n", motd->msg, motd->nick);

}

void cmd_list(int uid)
{
    for (int i = 0; i <= maxcli && client[i]->mode != 0; i++) {
        if (i % 5 == 0 && i != 0) {
            server_send(ONLY, uid, "%c", '\n');
        }
        server_send(ONLY, uid, "%s\t ", client[i]->nick);
    }
    server_send(ONLY, uid, "%c", '\n');
}
