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

#define _GNU_SOURCE

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/kchat.h"

void cmd_dm(int id, int argc, char *argv[])
{
	if (argc != 3) {
		server_send(ONLY, -1, id, "\r\e[33m * Usage: /dm nick \"msg\"\e[0m\n");
		return;
	}
	int to_id;
	if ((to_id = resolve_nick(argv[1])) == -1) {
		server_send(ONLY, -1, id, "\r\e[31m * User doesn't exist!\e[0m\n");
		return;
	}
	server_send(ONLY, id, to_id, "\r\e[1;%dm%s\e[0m> %s\n", clients[id]->color, clients[id]->nick, argv[2]);
}

static int cmpstringp(const void *p1, const void *p2)
{
	return strcmp(*(const char **) p1, *(const char **) p2);
}

void cmd_users(int id)
{
	char *users[connected];
	int j = 0;
	for (int i = 0; i < maxclients; i++)
		if (clients[i] != NULL)
			users[j++] = clients[i]->nick;

	qsort(users, j, sizeof(char *), cmpstringp);

	server_send(ONLY, -1, id, "\r\e[34m * Connected: %d", connected);
	for (int i = 0; i < connected; i++)
		server_send(ONLY, -1, id, "\n *	 %s", users[i]);
	server_send(ONLY, -1, id, "\e[0m\n");
}

static int nick_valid(int id, char *nick)
{
	int l = strlen(nick);
	if (l > 16) {
		server_send(ONLY, -1, id, "\r\e[33m * Too long (more than 16 characters)!\e[0m\n");
		return 1;
	}

	for (int i = 0; i < l; i++)
		if (!(isalnum(nick[i]) || nick[i] == '_')) {
			server_send(ONLY, -1, id, "\r\e[33m * Only 0-9, A-Z, a-z and underscore characters are allowed!\e[0m\n");
			return 1;
		}

	if (strncmp(nick, "guest", 5) == 0) {
		server_send(ONLY, -1, id, "\r\e[33m * Forbidden nickname!\e[0m\n");
		return 1;
	}

	return 0;
}

void cmd_nick(int id, int argc, char *argv[])
{
	if (argc != 2) {
		server_send(ONLY, -1, id, "\r\e[33m * Usage: /nick nickname\e[0m\n");
		return;
	}

	if (nick_valid(id, argv[1]))
		return;

	if (nick_handle(EXISTS, argv[1], NULL)) {
		server_send(ONLY, -1, id, "\r\e[33m * Already registered! Provide a password with /login command.\e[0m\n");
		return;
	}

	change_nick(DEFAULT, id, argv[1]);
}

void cmd_register(int id, int argc, char *argv[])
{
	if (argc != 3) {
		server_send(ONLY, -1, id, "\r\e[33m * Usage: /register nickname password\e[0m\n");
		return;
	}

	if (nick_valid(id, argv[1]))
		return;

	if (!nick_handle(REGISTER, argv[1], argv[2])) {
		server_send(ONLY, -1, id, "\r\e[33m * Already taken! Provide a password with /login command.\e[0m\n");
		return;
	}

	server_send(ONLY, -1, id, "\r\e[34m * Successfully registered.\e[0m\n");
	change_nick(REGISTER, id, argv[1]);
}

void cmd_unregister(int id)
{
	if (!nick_handle(REMOVE, clients[id]->nick, NULL)) {
		server_send(ONLY, -1, id, "\r\e[34m * Not registered!\e[0m\n");
		return;
	}

	server_send(ONLY, -1, id, "\r\e[34m * Successfully unregistered.\e[0m\n");
	char nick[17];
	snprintf(nick, 16, "guest_%d", id);
	change_nick(DEFAULT, id, nick);
}

void cmd_login(int id, int argc, char *argv[])
{
	if (argc != 3) {
		server_send(ONLY, -1, id, "\r\e[33m * Usage: /login nickname password\e[0m\n");
		return;
	}

	if (nick_valid(id, argv[1]))
		return;

	if (!nick_handle(LOGIN, argv[1], argv[2])) {
		server_send(ONLY, -1, id, "\r\e[33m * Wrong password or not registered!\e[0m\n");
		return;
	}

	change_nick(DEFAULT, id, argv[1]);
}

void cmd_rules(int id, int argc, char *argv[])
{
	if (argc != 3) {
		server_send(ONLY, -1, id,
			"\r\e[33m * Usage: /rules nickname mode\n"
			" * Modes:\n"
			" *   0. None\n"
			" *   1. Outgoing\n"
			" *   2. Incoming\n"
			" *   3. Default (outgoing + incoming)"
			"\e[0m\n");
		return;
	}

	int handle_id = resolve_nick(argv[1]);
	if (handle_id == -1) {
		server_send(ONLY, -1, id, "\r\e[31m * User doesn't exist!\e[0m\n");
		return;
	}

	if (handle_id == id) {
		server_send(ONLY, -1, id, "\r\e[31m * Can't change rules for yourself!\e[0m\n");
		return;
	}

	int mode = atoi(argv[2]);
	if (mode < 0 || mode > 3) {
		server_send(ONLY, -1, id, "\r\e[31m * Invalid mode!\e[0m\n");
		return;
	}

	clients[id]->ruleset[handle_id] = mode;
	server_send(ONLY, -1, id, "\r\e[34m * Rules for %s were changed to %d.\e[0m\n", argv[1], mode);
}

void cmd_help(int id)
{
	server_send(ONLY, -1, id,
		"\r\e[34m * Commands available:\n"
		" * /dm\n"
		" * /login\n"
		" * /nick\n"
		" * /restart\n"
		" * /register\n"
		" * /rules\n"
		" * /unregister\n"
		" * /users"
		"\e[0m\n");
}
