#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include "config.h"
#include "server.h"
#include "filehandler.h"
#include "common.h"

void remove_nl(char *str)
{
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n' || str[i] == '\r') {
            str[i] = '\0';
            break;
        }
    }
}

int splitarg(char str[], char cmd[], char arg[])
{
    int i, l = 0, hit = 0;
    for (i = 0; str[i] != '\0'; i++) {
        /* A command shall not contain space */
        if (!hit) {
            if (str[i] == ' ') {
                hit = 1;
                cmd[i] = '\0';
                continue;
            }
            cmd[i] = str[i];
        }
        // TODO: add size checks to prevent bof
        else {
            arg[l] = str[i];
            l++;
        }
    }
    /* 0 will be returned if no arguments are given */
    return hit;
}

int resolve_nick(char *nick)
{
    for (int i = 0; i < maxcli; i++) {
        if (strncmp(client[i]->nick, nick, 16) == 0) {
            return i;
        }
    }
    /* Queried nick didn't match any. */
    return -1;
}

/*
 * Sending mode:
 * 0. Send only to uid
 * 1. Send to everyone except uid
 * 2. Send to everyone (ignores uid)
 */
void server_send(int mode, int uid, const char *format, ...)
{
    char buf[outbufsize];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, outbufsize, format, args);
    va_end(args);

    if (mode == 0) {
        write(client[uid]->connfd, buf, strlen(buf));
    } else {
        for (int i = 0; i < maxcli; i++) {
            if (client[i]->mode > BIN) {
                if (mode == 2 || i != uid) {
                    write(client[i]->connfd, buf, strlen(buf));
                }
            }
        }
    }
}
