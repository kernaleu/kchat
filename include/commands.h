/*
 * Copyright (C) 2021 İrem Kuyucu <siren@kernal.eu>
 * Copyright (C) 2021 Laurynas Četyrkinas <stnby@kernal.eu>
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

#ifndef COMMANDS_H
#define COMMANDS_H

void cmd_dm(int id, int argc, char *argv[]);
void cmd_nick(int id, int argc, char *argv[]);
void cmd_users(int id);
void cmd_register(int id, int argc, char *argv[]);
void cmd_login(int id, int argc, char *argv[]);
void cmd_unregister(int id);
void cmd_rules(int id, int argc, char *argv[]);
void cmd_help(int id);

#endif
