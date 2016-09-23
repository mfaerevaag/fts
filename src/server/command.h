#ifndef COMMAND_H
#define COMMAND_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "config.h"
#include "logger.h"
#include "user.h"

struct command_t {
    char **chain;
    int len;
    int slot;
};
typedef struct command_t command;

command *cmd_decode(command *cmd, char buffer[BUF_SIZE]);
void cmd_handle(char buffer[BUF_SIZE], int slot, user **socks);

#endif
