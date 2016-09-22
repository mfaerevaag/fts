#include "command.h"

char *cmd_nick(command *cmd);

command *cmd_decode(char buffer[BUF_SIZE])
{
    command *cmd = malloc(sizeof(command));
    char copy[BUF_SIZE];

    sprintf(copy, "%s", buffer);

    cmd->chain = (char **) malloc(1 * sizeof(char*));

    int len = 0;
    int l = 0;
    int pos = 0;

    char *pch;
    pch = strtok(copy, " ");

    while (pch != NULL) {
        len++;
        cmd->chain = (char**) realloc(cmd->chain, (len * sizeof(char*)));

        if (len < 3) {
            l = (int) strlen(pch);
            pos = pos + l + 1;

            cmd->chain[len - 1] = (char *) malloc((l + 1) * sizeof(char));
            sprintf(cmd->chain[len - 1], "%s", pch);

        } else {
            cmd->chain[len - 1] = (char *) malloc(((int) strlen(&buffer[pos]) + 1) * sizeof(char));
            sprintf(cmd->chain[len - 1], "%s", &buffer[pos]);
            break;
        }

        pch = strtok(NULL, " ");
    }

    cmd->len = len;

    return cmd;
}

char *cmd_handle(command *cmd)
{
    if (strcmp(cmd->chain[0], "/nick") == 0) {
        return cmd_nick(cmd);
    } else {
        return "unknown command";
    }

    return NULL;
}

char *cmd_nick(command *cmd)
{
    /* check args */
    if (cmd->len != 2) {
        return "WARNING: usage: /nick <name>";
    }

    /* if(list_clients(cmd.chain[1])) */
    /*     return "WARNING: nick taken"; */

    /* client c = new_client(cmd.chain[1], cmd.nsd); */
    /* list_clients(c); */

    return "TODO: /nick";
}
