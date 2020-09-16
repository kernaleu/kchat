#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../include/kchat.h"
#include "../include/commands.h"

void cmd_dm(int id, int argc, char *argv[])
{
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

void cmd_users(int id)
{
    //  (16 + 2) * connected - 1
    // ", username1, username2, username3"
    char users[18 * connected];
    users[0] = '\0';

    for (int i = 0; i < maxclients; i++)
        if (clients[i]->connfd != -1) {
            if (users[0] != '\0')
                strcat(users, ", ");
            strcat(users, clients[i]->nick);
        }
    server_send(ONLY, id, "\r\e[34m * Users (connected: %d): %s.\e[0m\n", connected, users);
}

int nick_exists(char *nick, char *hash)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;

    if ((fp = fopen(AUTH_FILE, "r")) != NULL) {
        while (getline(&line, &len, fp) != -1) {
            if (strstr(line, nick)) {
                if (hash != NULL) {
                    strcpy(hash, line);
                    hash[strlen(hash)-1] = '\0';
                }
                return 1;
            }
        }
    }
    return 0;
}

void cmd_nick(int id, int argc, char *argv[])
{
    if (argc != 2) {
        server_send(ONLY, id, "\r\e[33m * Usage: /nick nickname\e[0m\n");
        return;
    }
    
    if (nick_exists(argv[1], NULL)) {
        server_send(ONLY, id, "\r\e[33m * This nickname is registered. Provide a password with /login.\e[0m\n");
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

void cmd_register(int id, int argc, char *argv[])
{
    if (argc != 3) {
        server_send(ONLY, id, "\r\e[33m * Usage: /register nickname password\e[0m\n");
        return;
    }
    
    if (strchr(argv[2], ':')) {
        server_send(ONLY, id, "\r\e[33m * Nicknames musn't contain ':'.\e[0m\n");
        return;
}

    if (nick_exists(argv[1], NULL)) {
        server_send(ONLY, id, "\r\e[33m * This nickname has already been registered. Provide a password with /login.\e[0m\n");
        return;
    }
    
    unsigned char ubytes[16];
    char salt[20];
    const char *const saltchars = "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    char *hash;

    if (getentropy(ubytes, sizeof ubytes)) {
        perror ("getentropy");
        return;
    }

    salt[0] = '$';
    salt[1] = '5'; /* SHA-256 */
    salt[2] = '$';
    for (int i = 0; i < 16; i++)
        salt[3+i] = saltchars[ubytes[i] & 0x3f];
    salt[19] = '\0';
    
    FILE *fp = fopen(AUTH_FILE, "a");
    fprintf(fp, "%s:%s\n", argv[1], crypt(argv[2], salt));
    fclose(fp);

    server_send(ONLY, id, "\r\e[34m * Succesfully registered nickname.\e[0m\n");
}

void cmd_login(int id, int argc, char *argv[])
{
    if (argc != 3) {
        server_send(ONLY, id, "\r\e[33m * Usage: /login nickname password\e[0m\n");
        return;
    }
    
    char buf[100];
    if (!nick_exists(argv[1], buf)) {
        server_send(ONLY, id, "\r\e[33m * Nickname isn't registered.\e[0m\n");
        return;
    }
    
    char *hash = strstr(buf, ":") + 1;
     
    if (strcmp(crypt(argv[2], hash), hash)) {
        server_send(ONLY, id, "\r\e[33m * Provided password didn't match.\e[0m\n");
        return;
    }

    char oldnick[17];
    strncpy(oldnick, clients[id]->nick, 16);
    oldnick[16]= '\0';

    if (change_nick(id, argv[1]))
        server_send(EVERYONE, 0, "\r\e[34m * %s is now known as %s.\e[0m\n", oldnick, clients[id]->nick);
}
