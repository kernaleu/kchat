/*
 * Copyright (c) 2010, 2011, 2012 Ryan Flannery <ryan.flannery@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "../include/str2argv.h"

#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

/* Initialize empty argc/argv struct. */
void argv_init(int *argc, char ***argv)
{
	if (NULL == (*argv = (char**) calloc(ARGV_MAX_ENTRIES, sizeof(char*))))
		err(1, "argv_init: argv calloc fail");

	if (NULL == ((*argv)[0] = (char*) calloc(ARGV_MAX_TOKEN_LEN, sizeof(char))))
		err(1, "argv_init: argv[i] calloc fail");

	memset((*argv)[0], 0, ARGV_MAX_TOKEN_LEN * sizeof(char));
	*argc = 0;
}

/* Free all memory in an arc/argv. */
void argv_free(int *argc, char ***argv)
{
	for (int i = 0; i <= *argc; i++)
		free((*argv)[i]);

	free(*argv);
	*argc = 0;
}

/* Add a character to the end of the current entry in an argc/argv. */
void argv_addch(int argc, char **argv, int c)
{
	int n = strlen(argv[argc]);
	if (ARGV_MAX_TOKEN_LEN - 1 == n)
		errx(1, "argv_addch: reached max token length (%d)", ARGV_MAX_TOKEN_LEN);

	argv[argc][n] = c;
}

/* Complete the current entry in the argc/argv and setup the next one. */
void argv_finish_token(int *argc, char ***argv)
{
	if (ARGV_MAX_ENTRIES - 1 == *argc)
		errx(1, "argv_finish_token: reached max argv entries(%d)", ARGV_MAX_ENTRIES);

	if (0 == strlen((*argv)[*argc]))
		return;

	*argc = *argc + 1;
	if (NULL == ((*argv)[*argc] = (char*) calloc(ARGV_MAX_TOKEN_LEN, sizeof(char))))
		err(1, "argv_finish_token: failed to calloc argv[i]");

	memset((*argv)[*argc], 0, ARGV_MAX_TOKEN_LEN * sizeof(char));
}

/*
 * Main parser used for converting a string (str) to an argc/argv style
 * parameter list. This handles escape sequences and quoting. Possibly
 * correctly. :D
 * The argc/argv parameters passed are over-written. After they have been
 * built by this function, the caller should use argv_free() on them to
 * free() all associated memory.
 * If the parsing goes correctly, 0 is returned. Otherwise, 1 is returned
 * and the errmsg parameter is set to some appropriate error message and
 * both argc/argv are set to 0/NULL.
 */
int str2argv(const char *str, int *argc, char ***argv, const char **errmsg)
{
	const char *ERRORS[2] = {
		"Unmatched quotes",
		"Unused/Dangling escape sequence"
	};

	if (NULL == argc || NULL == argv || NULL == errmsg)
		return 1;

	*errmsg = NULL;

	char container_start = 0;
	bool in_token = false;
	bool in_container = false;
	bool escaped = false;
	int len = strlen(str);

	argv_init(argc, argv);
	for (int i = 0; i < len; i++) {
		char c = str[i];
		switch (c) {
		/* Handle whitespace. */
		case ' ':
		case '\t':
		case '\n':
			if (!in_token)
				continue;

			if (in_container) {
				argv_addch(*argc, *argv, c);
				continue;
			}

			if (escaped) {
				escaped = false;
				argv_addch(*argc, *argv, c);
				continue;
			}

			/* If reached here, we're at end of token. */
			in_token = false;
			argv_finish_token(argc, argv);
			break;

		/* Handle quotes. */
		case '\'':
		case '\"':
			if (escaped) {
				argv_addch(*argc, *argv, c);
				escaped = false;
				continue;
			}
			if (!in_token) {
				in_token = true;
				in_container = true;
				container_start = c;
				continue;
			}
			if (in_token && !in_container) {
				in_container = true;
				container_start = c;
				continue;
			}
			if (in_container) {
				if (c == container_start) {
					in_container = false;
					in_token = false;
					argv_finish_token(argc, argv);
					continue;
				} else {
					argv_addch(*argc, *argv, c);
					continue;
				}
			}
			*errmsg = ERRORS[0];
			argv_free(argc, argv);
			return 1;
		case '\\':
			if (in_container && str[i+1] != container_start) {
				argv_addch(*argc, *argv, c);
				continue;
			}
			if (escaped) {
				escaped = false;
				argv_addch(*argc, *argv, c);
				continue;
			}

			escaped = true;
			break;
		default:
			if (!in_token)
				in_token = true;

			if (escaped)
				escaped = false;

			argv_addch(*argc, *argv, c);
		}
	}
	argv_finish_token(argc, argv);

	if (in_container) {
		argv_free(argc, argv);
		*errmsg = ERRORS[0];
		return 1;
	}
	if (escaped) {
		argv_free(argc, argv);
		*errmsg = ERRORS[1];
		return 1;
	}
	if ((*argv)[*argc] != NULL) {
		free((*argv)[*argc]);
		(*argv)[*argc] = NULL;
	}
	return 0;
}

char *argv2str(int argc, char *argv[])
{
	char *result;

	/* Handle empty case. */
	if (0 >= argc)
		return NULL;

	/* Determine length of resulting string. */
	int len = 0;
	for (int i = 0; i < argc; i++) {
		len += strlen(argv[i]) + 1;
		if (NULL != strstr(argv[i], " "))
		len += 2;
	}

	/* Allocate result. */
	if (NULL == (result = (char*) calloc(len, sizeof(char))))
		err(1, "argv2str: calloc failed");
	memset(result, 0, len);

	/* Build result. */
	int off = 0;
	for (int i = 0; i < argc; i++) {
		if (NULL == strstr(argv[i], " "))
			off += snprintf(result + off, len, "%s", argv[i]);
		else
			off += snprintf(result + off, len, "\'%s\'", argv[i]);

		if (i < argc - 1)
			off += snprintf(result + off, len, " ");
	}
	return result;
}
