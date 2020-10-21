#define _GNU_SOURCE

#include <stdio.h>
#include <ctype.h>
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
        if (clients[i] != NULL) {
            if (users[0] != '\0')
                strcat(users, ", ");
            strcat(users, clients[i]->nick);
        }
    server_send(ONLY, id, "\r\e[34m * Users (connected: %d): %s.\e[0m\n", connected, users);
}

static int nick_exists(char *nick, char *hash)
{
    int ret = 0;
    FILE *fp = fopen(AUTH_FILE, "r");

    if (fp != NULL) {
        char *line = NULL, delim[] = ":";
        size_t len = 0;
        ssize_t nread;
        while ((nread = getline(&line, &len, fp)) != -1) {
            line[nread - 1] = '\0';
            char *token = strtok(line, delim);
            //printf("(serv) strtok nick = \"%s\"\n", token);
            if (strcmp(nick, token) == 0) {
                if (hash != NULL) {
                    token = strtok(NULL, delim);
                    //printf("(serv) strtok hash = \"%s\"\n", token);
                    strcpy(hash, token);
                }
                ret = 1;
                break;
            }
        }
        free(line);
        fclose(fp);
    }
    return ret;
}

static int username_valid(int id, char *nick)
{
    if (strlen(nick) > 16) {
        server_send(ONLY, id, "\r\e[33m * This nickname is too long (more than 16 characters).\e[0m\n");
        return 1;
    }
    for (int i = 0; i < strlen(nick); i++)
        if (!(isalnum(nick[i]) || nick[i] == '_')) {
            server_send(ONLY, id, "\r\e[33m * Only 0-9, A-Z, a-z and underscores are allowed for nicknames.\e[0m\n");
            return 1;
        }
    if (strncmp(nick, "guest", 5) == 0) {
        server_send(ONLY, id, "\r\e[33m * Forbidden nickname.\e[0m\n");
        return 1;
    }
    return 0;
}

void cmd_nick(int id, int argc, char *argv[])
{
    if (argc != 2) {
        server_send(ONLY, id, "\r\e[33m * Usage: /nick nickname\e[0m\n");
        return;
    }

    if (username_valid(id, argv[1]))
        return;

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

    if (username_valid(id, argv[1]))
        return;

    if (nick_exists(argv[1], NULL)) {
        server_send(ONLY, id, "\r\e[33m * This nickname has already been registered. Provide a password with /login.\e[0m\n");
        return;
    }

    unsigned char ubytes[16];
    char salt[20];
    const char *const saltchars = "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    if (getentropy(ubytes, sizeof ubytes)) {
        perror("getentropy");
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

    server_send(ONLY, id, "\r\e[34m * Successfully registered nickname.\e[0m\n");
    change_nick(id, argv[1]);
}

void cmd_login(int id, int argc, char *argv[])
{
    if (argc != 3) {
        server_send(ONLY, id, "\r\e[33m * Usage: /login nickname password\e[0m\n");
        return;
    }

    if (username_valid(id, argv[1]))
        return;

    char hash[64];
    if (!nick_exists(argv[1], hash)) {
        server_send(ONLY, id, "\r\e[33m * Nickname isn't registered.\e[0m\n");
        return;
    }

    if (strcmp(crypt(argv[2], hash), hash) != 0) {
        server_send(ONLY, id, "\r\e[33m * Provided password didn't match.\e[0m\n");
        return;
    }

    char oldnick[17];
    strcpy(oldnick, clients[id]->nick);

    if (change_nick(id, argv[1]))
        server_send(EVERYONE, 0, "\r\e[34m * %s is now known as %s.\e[0m\n", oldnick, clients[id]->nick);
}
