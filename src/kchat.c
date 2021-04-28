#define _GNU_SOURCE

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../include/commands.h"
#include "../include/kchat.h"
#include "../include/str2argv.h"

static int bufsize = BUF_SIZE;
static char *motd = MOTD;

client_t *clients[MAX_CLIENTS];
int sockfd;
int maxclients = MAX_CLIENTS;
int connected = 0;

void quit()
{
	puts("\r(serv) Shutting down...");
	server_send(EVERYONE, -1, -1, "\r\e[34m * Server is shutting down...\e[0m\n");
	for (int id = 0; id < maxclients; id++) {
		if (clients[id] != NULL) {
			close(clients[id]->connfd);
			free(clients[id]);
		}
	}
	close(sockfd);
	exit(0);
}


static void client_disconnect(int id)
{
	close(clients[id]->connfd);
	connected--;

	server_send(EXCEPT, -1, id, "\r\e[34m * %s left. (connected: %d)\e[0m\n",
	    clients[id]->nick, connected);
	free(clients[id]);
	clients[id] = NULL;

	/* Set default rule on all clients for the disconnecting user. */
	for (int i = 0; i < maxclients; i++)
		if (clients[i] != NULL)
			clients[i]->ruleset[id] = 3;
}

int main()
{
	int connfd, id;
	fd_set descriptors;
	char buf[bufsize + 1]; /* 1 more to leave space for '\0'. */

	for (id = 0; id < maxclients; id++)
		clients[id] = NULL;

	struct sockaddr_in serv_addr, cli_addr;
	int addrlen = sizeof(cli_addr);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return 1;
	}

	int option = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT);

	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("bind");
		return 1;
	}

	if (listen(sockfd, BACKLOG) < 0) {
		perror("listen");
		return 1;
	}

	signal(SIGPIPE, SIG_IGN); /* Ignore SIGPIPE as we handle it manually. */
	signal(SIGINT, quit);

	puts("(serv) Waiting for connections...");
	for (;;) {
		FD_ZERO(&descriptors);
		FD_SET(sockfd, &descriptors);
		int maxfd = sockfd;
		/* Add all socket descriptors to the read list. */
		for (id = 0; id < maxclients; id++) {
			if (clients[id] != NULL) {
				FD_SET(clients[id]->connfd, &descriptors);
				/* Find highest file descriptor, needed for the select function. */
				if (clients[id]->connfd > maxfd)
					maxfd = clients[id]->connfd;
			}
		}

		if (select(maxfd + 1 ,&descriptors, NULL, NULL, NULL) == -1)
			perror("select");
		/* Incoming connection on the primary socket. (new client) */
		if (FD_ISSET(sockfd, &descriptors)) {
			if ((connfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t*)&addrlen)) == -1) {
				perror("accept");
				exit(1);
			}
			printf("(serv) New connection, sockfd: %d, ipaddr: %s, port: %d\n", connfd,
			    inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
			for (id = 0; id < maxclients; id++) {
				/* If position is empty. */
				if (clients[id] == NULL) {
					clients[id] = malloc(sizeof(client_t));
					clients[id]->connfd = connfd;
					clients[id]->color = rand() % 5 + 31;
					/* Set default rules for the clients. */
					memset(clients[id]->ruleset, 3, sizeof(int) * maxclients);
					snprintf(clients[id]->nick, 16, "guest_%d", id);
					connected++;
					server_send(EXCEPT, -1, id, "\r\e[34m * %s joined. (connected: %d)\e[0m\n",
					    clients[id]->nick, connected);
					server_send(ONLY, -1, id, "%s\n", motd);
					break;
				}
			}
			if (id == maxclients) {/* Server is full. */
				puts("(serv) Server is full. Disconnecting.");
				close(connfd);
			}
		}
		/* IO operations on other sockets. */
		for (id = 0; id < maxclients; id++) {
			if (clients[id] != NULL && FD_ISSET(clients[id]->connfd, &descriptors)) {
				ssize_t bytesread;
				if ((bytesread = read(clients[id]->connfd, buf, bufsize)) > 0) {
					buf[bytesread] = '\0';
					trim(buf);

					/* Skip empty messages. */
					if (strlen(buf) == 0)
						continue;
					/* Handle commands. */
					if (buf[0] == '/')
						command_handle(id, buf);
					/* Send message. */
					else
						server_send(EXCEPT, id, id, "\r\e[1;%dm%s\e[0m: %s\n",
						    clients[id]->color, clients[id]->nick, buf);
				}
				/* Client disconnected. */
				else {
					puts("(serv) Client disconnected.");
					client_disconnect(id);
				}
			}
		}
	}
	return 0;
}

static int check_rules(int from_id, int to_id)
{
	/* If receiver is invalid just stop over here. */
	if (clients[to_id] == NULL)
		return 0;

	/* Server messages are always permitted. */
	if (from_id < 0)
		return 1;

	/* If sender wants to send and receiver wants to receive. */
	if (clients[from_id] != NULL &&
	    clients[from_id]->ruleset[to_id] % 2 &&
	    clients[to_id]->ruleset[from_id] > 1)
		return 1;
	return 0;
}

static ssize_t server_write(int id, const void *buf, size_t count)
{
	ssize_t ret;
	if ((ret = write(clients[id]->connfd, buf, count)) == -1) {
		if (errno == EPIPE) {
			puts("(serv) Client disconnected. (epipe)");
			client_disconnect(id);
		} else {
			puts("(serv) Client disconnected. (¯\\_(ツ)_/¯)");
			perror("write");
		}
	}
	return ret;
}

/*
 * Sending mode:
 *   0. Send only to id
 *   1. Send to everyone except to_id
 *   2. Send to everyone (ignore to_id)
 */
void server_send(int mode, int from_id, int to_id, const char *fmt, ...)
{
	char *str;
	va_list ap;
	va_start(ap, fmt);
	int len = vasprintf(&str, fmt, ap);
	va_end(ap);

	if (mode == ONLY && check_rules(from_id, to_id))
		server_write(to_id, str, len);
	else
		for (int i = 0; i < maxclients; i++)
			if ((mode == EVERYONE || to_id != i) && check_rules(from_id, i))
				server_write(i, str, len);
	free(str);
}

int resolve_nick(char *nick)
{
	for (int id = 0; id < maxclients; id++)
		if (clients[id] != NULL && strcmp(clients[id]->nick, nick) == 0)
			return id;
	/* Queried nick didn't match any. */
	return -1;
}

static char *hash_pass(const char *pass)
{
	char salt[20] = "$5$";
	const char *const saltchars = "./0123456789ABCDEFGHIJKLMNOPQRST"
	    "UVWXYZabcdefghijklmnopqrstuvwxyz";

	/* Retrieve 16 random bytes from the operating system. */
	unsigned char ubytes[16];
	if (getentropy (ubytes, sizeof ubytes)) {
		perror ("getentropy");
		return NULL;
	}

	for (int i = 0; i < 16; i++)
		salt[3 + i] = saltchars[ubytes[i] & 0x3f];
	salt[19] = '\0';

	return crypt(pass, salt);
}

int change_nick(int mode, int id, char *nick)
{
	if (mode && strcmp(clients[id]->nick, nick) == 0) /* In case user tries to register currently set nickname. */
		return 1;

	else if (resolve_nick(nick) != -1) {
		server_send(ONLY, -1, id, "\r\e[33m * Already logged in!\e[0m\n");
		return 0;
	}
	server_send(EVERYONE, -1, -1, "\r\e[34m * %s is now known as %s.\e[0m\n", clients[id]->nick, nick);
	strcpy(clients[id]->nick, nick);
	return 1;
}

/*
 * Handling mode:
 *   0 (EXISTS). Check if nick is registered (pass set to NULL).
 *   1 (LOGIN). Authenticate if provided pass matches the registered hash.
 *   2 (REGISTER). Register if nick is available.
 *   3 (REMOVE). Remove (pass set to NULL).
 */
int nick_handle(int mode, char *nick, char *pass)
{
	int ret = 0, found = 0;
	FILE *fp[2];
	fp[0] = fopen(AUTH_FILE, "a+");
	char *line = NULL;
	char delim[] = ":";
	size_t len;
	ssize_t chars;
	unsigned lineno[2];
	for (lineno[0] = 0; (chars = getline(&line, &len, fp[0])) != -1; lineno[0]++) {
		if (line[chars - 1] == '\n')
			line[chars - 1] = '\0';
		char *token = strtok(line, delim);
		if (strcmp(nick, token) == 0) { /* Found a line with our nick. */
			found = 1;
			switch (mode) {
			case EXISTS:
				ret = 1;
				break;
			case LOGIN:
				token = strtok(NULL, delim);
				if (strcmp(crypt(pass, token), token) == 0) /* Password hashes match. */
					ret = 1;
				break;
			case REMOVE:
				fp[1] = fopen(AUTH_FILE_TMP, "w");
				rewind(fp[0]);
				/*
				 * Read line by line old file and write to a new temporary file,
				 * except the line we want to remove.
				 */
				for (lineno[1] = 0; (chars = getline(&line, &len, fp[0])) != -1; lineno[1]++) {
					if (lineno[0] == lineno[1])
						break;
					fwrite(line, 1, chars, fp[1]);
				}
				/*
				 * Write the remaining part of the file, but here we don't
				 * need to count lines anymore nor check them.
				 */
				while ((chars = getline(&line, &len, fp[0])) != -1)
					fwrite(line, 1, chars, fp[1]);
				fclose(fp[1]);
				rename(AUTH_FILE_TMP, AUTH_FILE); /* Replace the original file with temporary one. */
				ret = 1;
			}
			break;
		}
	}
	free(line);
	if (mode == REGISTER && !found) {
		fprintf(fp[0], "%s:%s\n", nick, hash_pass(pass));
		ret = 1;
	}
	fclose(fp[0]);
	return ret;
}

/* Remove leading and trailing white space characters. */
void trim(char *str)
{
	int i, j;

	/* End string on EOL. */
	for (i = 0; str[i] != '\0'; i++)
		if (str[i] == '\n' || str[i] == '\r') {
			str[i] = '\0';
			break;
		}

	/* Trim leading white spaces. */
	for (i = 0; isspace(str[i]); i++);

	/* Shift all trailing characters to its left. */
	for (j = 0; str[i + j] != '\0'; j++)
		str[j] = str[i + j];
	str[j] = '\0'; /* Terminate string with NULL. */

	/* Trim trailing white spaces. */
	i = -1;
	for (j = 0; str[j] != '\0'; j++)
		if (!isspace(str[j]))
			i = j;

	/* Set trailing character to NULL. */
	str[i + 1] = '\0';
}

void command_handle(int id, char *str)
{
	const char *errmsg;
	char **argv;
	int argc;

	if (str2argv(str, &argc, &argv, &errmsg) != 0) {
		server_send(ONLY, -1, id, "\r\e[31m * %s!\e[0m\n", errmsg);
		return;
	}

	if (strcmp("/nick", argv[0]) == 0)
		cmd_nick(id, argc, argv);
	else if (strcmp("/dm", argv[0]) == 0)
		cmd_dm(id, argc, argv);
	else if (strcmp("/users", argv[0]) == 0)
		cmd_users(id);
	else if (strcmp("/restart", argv[0]) == 0)
		quit();
	else if (strcmp("/register", argv[0]) == 0)
		cmd_register(id, argc, argv);
	else if (strcmp("/unregister", argv[0]) == 0)
		cmd_unregister(id);
	else if (strcmp("/login", argv[0]) == 0)
		cmd_login(id, argc, argv);
	else if (strcmp("/rules", argv[0]) == 0)
		cmd_rules(id, argc, argv);
	else if (strcmp("/help", argv[0]) == 0)
		cmd_help(id);
	else
		server_send(ONLY, -1, id, "\r\e[31m * Unknown command!\e[0m\n");
	argv_free(&argc, &argv);
}
