#ifndef COMMANDS_H
#define COMMANDS_H

void cmd_dm(int id, int argc, char *argv[]);
void cmd_nick(int id, int argc, char *argv[]);
void cmd_users(int id);
void cmd_register(int id, int argc, char *argv[]);
void cmd_login(int id, int argc, char *argv[]);
void cmd_unregister(int id, int argc, char *argv[]);
void cmd_rules(int id, int argc, char *argv[]);

#endif
