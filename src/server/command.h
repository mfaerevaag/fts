#ifndef COMMAND_H
#define COMMAND_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

struct command_t {
    char **chain;
    int len;
    int nsd;
};
typedef struct command_t command;

command *cmd_decode(char *buffer);
char *cmd_handle(command *cmd);

#endif
