#include <stdio.h>
#include <string.h>
#include "../include/kchat.h"
#include "../include/commands.h"

void cmd_dm(int id, int argc, char *argv[]) {
    if (argc != 3) {
        server_send(ONLY, id, "\r\e[33m * Usage: /dm nick \"msg\"\e[0m\n");
        return;
    }
    int to_id;
    if ((to_id = resolve_nick(argv[1])) == -1) {
        server_send(ONLY, id, "\r\e[31m * User with this nick does not exist!\e[0m\n");
        return;
    }
    server_send(ONLY, to_id, "\r\e[1;%dm%s\e[0m> %s\n", clients[id]->color, clients[id]->nick, argv[2]);
}

void cmd_nick(int id, int argc, char *argv[]) {
    if (argc != 2) {
        server_send(ONLY, id, "\r\e[33m * Usage: /nick nickname\e[0m\n");
        return;
    }
    char oldnick[17];
    strncpy(oldnick, clients[id]->nick, 16);
    oldnick[16]= '\0';

    if (change_nick(id, argv[1]))
        server_send(EVERYONE, 0, "\r\e[34m * %s is now known as %s.\e[0m\n", oldnick, clients[id]->nick);
    else
        server_send(ONLY, id, "\r\e[31m * User with this nick is already logged in.\e[0m\n");
}