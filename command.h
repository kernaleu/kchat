void cmd_nick(int uid, char *nick);
void list_users(int uid);
int is_registered(FILE *fp, char *nick, char *line);
void nick_reg(int uid, char *nick);
void remove_nl(char *arg);
