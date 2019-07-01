#include <stdio.h>
#include <string.h>
#include "client.h"
#include "command.h"

void remove_nl(char *arg)
{
    for (int i = 0; arg[i] != '\0'; i++) {
        if (arg[i] == '\n' || arg[i] == '\r')
            arg[i] = '\0';
    }                                            
}

int change_name(char *arg, int id)
{
    char tmp[1024];
    remove_nl(arg);
    strcpy(client[id]->uname, arg);
    sprintf(tmp, "** %d is now known as %s\n", client[id]->id, client[id]->uname);
    send_all(tmp);
}
