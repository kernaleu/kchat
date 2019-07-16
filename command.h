void cmd_nick(int uid, char *nick);
void list_users(int uid);
int is_registered(int fd, char *nick, char *line);
void nick_reg(int uid, char *nick);
void direct_msg(int uid, char *arg);
void remove_nl(char *arg);
