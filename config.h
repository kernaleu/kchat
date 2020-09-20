#ifndef CONFIG_H
#define CONFIG_H

#define PORT 1337
#define BACKLOG 10 /* Pending connections in queue */
#define BUF_IN_SIZE 512
#define MAX_CLIENTS 17
#define AUTH_FILE "auth.txt"
#define MOTD  "\r\e[1;34m" \
"                __\n" \
"             -=(o '.\n" \
" \e[3mKernal   \e[1m      '.-.\\\n" \
" \e[3mCommunity   \e[1m   /|  \\\\\n" \
" \e[3m2017-present\e[1m   '|  ||\n" \
"                _\\_):,_" \
"\e[0m\n"


#endif
