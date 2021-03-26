#define _GNU_SOURCE

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../include/kchat.h"
#include "../include/commands.h"

void cmd_dm(int id, int argc, char *argv[])
{
	if (argc != 3) {
		server_send(ONLY, -1, id, "\r\e[33m * Usage: /dm nick \"msg\"\e[0m\n");
		return;
	}
	int to_id;
	if ((to_id = resolve_nick(argv[1])) == -1) {
		server_send(ONLY, -1, id, "\r\e[31m * User with this nick does not exist!\e[0m\n");
		return;
	}
	server_send(ONLY, id, to_id, "\r\e[1;%dm%s\e[0m> %s\n", clients[id]->color, clients[id]->nick, argv[2]);
}

static int cmpstringp(const void *p1, const void *p2)
{
	return strcmp(* (char * const *) p1, * (char * const *) p2);
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

static int nick_exists(char *nick, char *hash)
{
	int ret = -1, lineno = 0;
	FILE *fp = fopen(AUTH_FILE, "r");

	if (fp != NULL) {
		char *line = NULL, delim[] = ":";
		size_t len = 0;
		ssize_t nread;
		while ((nread = getline(&line, &len, fp)) != -1) {
			line[nread - 1] = '\0';
			char *token = strtok(line, delim);
			if (strcmp(nick, token) == 0) {
				ret = lineno;
				if (hash != NULL) {
					token = strtok(NULL, delim);
					strcpy(hash, token);
				}
				break;
			}
			lineno++;
		}
		free(line);
		fclose(fp);
	}
	return ret;
}

static int username_valid(int id, char *nick)
{
	if (strlen(nick) > 16) {
		server_send(ONLY, -1, id, "\r\e[33m * This nickname is too long (more than 16 characters).\e[0m\n");
		return 1;
	}

	for (int i = 0; i < strlen(nick); i++)
		if (!(isalnum(nick[i]) || nick[i] == '_')) {
			server_send(ONLY, -1, id, "\r\e[33m * Only 0-9, A-Z, a-z and underscores are allowed for nicknames.\e[0m\n");
			return 1;
		}

	if (strncmp(nick, "guest", 5) == 0) {
		server_send(ONLY, -1, id, "\r\e[33m * Forbidden nickname.\e[0m\n");
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

	if (username_valid(id, argv[1]))
		return;

	if (nick_exists(argv[1], NULL) > 0) {
		server_send(ONLY, -1, id, "\r\e[33m * This nickname is registered. Provide a password with /login.\e[0m\n");
		return;
	}

	char oldnick[17];
	strcpy(oldnick, clients[id]->nick);

	if (change_nick(id, argv[1]))
		server_send(EVERYONE, -1, -1, "\r\e[34m * %s is now known as %s.\e[0m\n", oldnick, clients[id]->nick);
	else
		server_send(ONLY, -1, id, "\r\e[31m * User with this nick is already logged in.\e[0m\n");
}

void cmd_register(int id, int argc, char *argv[])
{
	if (argc != 3) {
		server_send(ONLY, -1, id, "\r\e[33m * Usage: /register nickname password\e[0m\n");
		return;
	}

	if (username_valid(id, argv[1]))
		return;

	if (nick_exists(argv[1], NULL) > 0) {
		server_send(ONLY, -1, id, "\r\e[33m * This nickname has already been registered. Provide a password with /login.\e[0m\n");
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

	server_send(ONLY, -1, id, "\r\e[34m * Successfully registered nickname.\e[0m\n");
	change_nick(id, argv[1]);
}

void cmd_unregister(int id, int argc, char *argv[])
{
	int lineno = nick_exists(clients[id]->nick, NULL);

	if (lineno < 0) {
		server_send(ONLY, -1, id, "\r\e[34m * Your current nickname isn't registered.\e[0m\n");
		return;
	}

	int fd;
	struct stat sb;
	char *addr;
	size_t length;
	ssize_t s;

	fd = open(AUTH_FILE, O_RDWR);
	if (fd == -1) {
		perror("open");
		return;
	}

	if (fstat(fd, &sb) == -1) {
		perror("fstat");
		return;
	}

	addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED) {
		perror("mmap");
		return;
	}

	int cur = 0;
	for (int i = 0; i <= sb.st_size; i++) {
		if (cur != (lineno))
			write(fd, &(addr[i]), 1);

		if (addr[i] == '\n')
			cur++;
	}

	ftruncate(fd, sb.st_size - (strlen(clients[id]->nick) + 65));

	server_send(ONLY, -1, id, "\r\e[34m * Successfully unregistered nickname.\e[0m\n");

	munmap(addr, sb.st_size);
	close(fd);
}

void cmd_login(int id, int argc, char *argv[])
{
	if (argc != 3) {
		server_send(ONLY, -1, id, "\r\e[33m * Usage: /login nickname password\e[0m\n");
		return;
	}

	if (username_valid(id, argv[1]))
		return;

	char hash[64];
	if (nick_exists(argv[1], hash) < 0) {
		server_send(ONLY, -1, id, "\r\e[33m * Nickname isn't registered.\e[0m\n");
		return;
	}

	if (strcmp(crypt(argv[2], hash), hash) != 0) {
		server_send(ONLY, -1, id, "\r\e[33m * Provided password didn't match.\e[0m\n");
		return;
	}

	char oldnick[17];
	strcpy(oldnick, clients[id]->nick);

	if (change_nick(id, argv[1]))
		server_send(EVERYONE, -1, -1, "\r\e[34m * %s is now known as %s.\e[0m\n", oldnick, clients[id]->nick);
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
			"\e[0m\n"
		);
		return;
	}

	int handle_id = resolve_nick(argv[1]);
	if (handle_id == -1) {
		server_send(ONLY, -1, id, "\r\e[31m * User with this nick does not exist!\e[0m\n");
		return;
	}

	if (handle_id == id) {
		server_send(ONLY, -1, id, "\r\e[31m * Can not change rules for yourself!\e[0m\n");
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
		" * /users\n"
		"\e[0m\n"
	);
	return;
}
