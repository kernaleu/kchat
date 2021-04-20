#ifndef KCHAT_H
#define KCHAT_H

#include "../config.h"

extern int maxclients;
extern int connected;

typedef struct {
	int connfd;
	int color;
	char nick[17];
	/*
	 * Modes:
	 *   None = 0
	 *   Outgoing = 1
	 *   Incoming = 2
	 *   Default (outgoing + incoming) = 3
	 */
	int ruleset[MAX_CLIENTS];
} client_t;

extern client_t *clients[];

#define ONLY 0
#define EXCEPT 1
#define EVERYONE 2
void server_send(int mode, int from_id, int to_id, const char *format, ...);
int resolve_nick(char *nick);
void trim(char *str);
#define DEFAULT 0
#define EXISTS 0
#define REGISTER 1
#define LOGIN 2
#define REMOVE 3
int nick_handle(int mode, char *nick, char *pass);
int change_nick(int mode, int id, char *str);
void command_handle(int id, char *str);

#endif
