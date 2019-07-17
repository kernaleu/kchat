void cmd_login(int uid, char *arg);
void cmd_list(int uid);
void cmd_dm(int uid, char *arg);
void cmd_register(int uid, char *arg); /* Placeholder*/

int find_pass(char *nick, char *pass);
void remove_nl(char *arg);
int resolve_nick(char *nick);
