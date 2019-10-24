#ifndef COMMON_H
#define COMMON_H

#define ONLY 0
#define EXCEPT 1
#define EVERYONE 2

void remove_nl(char *str);
int splitarg(char *str, char *cmd, char *arg); 
int resolve_nick(char *nick);
void server_send(int mode, int uid, const char *format, ...);

#endif
