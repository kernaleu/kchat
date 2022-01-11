/*
 * Copyright (C) 2022 İrem Kuyucu <siren@kernal.eu>
 * Copyright (C) 2022 Laurynas Četyrkinas <stnby@kernal.eu>
 *
 * This file is part of Kernal Chat.
 *
 * Kernal Chat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kernal Chat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kernal Chat.  If not, see <https://www.gnu.org/licenses/>.
 */

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
