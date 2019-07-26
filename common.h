#ifndef COMMON_H
#define COMMON_H

#define ONLY 0
#define EXCEPT 1
#define EVERYONE 2

void remove_nl(char *str);
int resolve_nick(char *nick);
void server_send(int mode, int uid, const char *format, ...);
void motd_set(char *nick, char *msg);
void motd_send();

#endif
