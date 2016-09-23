#include "command.h"

char *cmd_nick(command *cmd);
char *cmd_broadcast(command *cmd, int clients[MAX_CONN]);

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

void cmd_handle(char buffer[BUF_SIZE], int slot, int clients[MAX_CONN])
{
    /* decode */
    command *cmd = cmd_decode(buffer);
    cmd->nsd = clients[slot];

    memset(buffer, 0, BUF_SIZE);

    if (cmd->chain[0] != NULL) {
        if (strcmp(cmd->chain[0], "/nick") == 0) {
            sprintf(buffer, "%s", cmd_nick(cmd));

        } else if (strcmp(cmd->chain[0], "/all") == 0) {
            sprintf(buffer, "%s", cmd_broadcast(cmd, clients));

        } else if (strcmp(cmd->chain[0], "/quit") == 0) {
            log_infof("slot %d quit\n", slot);
            clients[slot] = 0;

        } else {
            log_warn("unknown command");
            sprintf(buffer, "unknown command");
        }

        int n = send(cmd->nsd, buffer, BUF_SIZE, 0);
        if (n < 0)
            perror("ERROR on responding");
    }

    free(cmd); // TODO: stack
}

char *cmd_nick(command *cmd)
{
    /* check args */
    if (cmd->len != 2) {
        return "usage: /nick <name>";
    }

    return "TODO: /nick";
}

char *cmd_broadcast(command *cmd, int clients[MAX_CONN])
{
    /* check args */
    if (cmd->len != 2) {
        return "usage: /all <msg>";
    }

    char buffer[BUF_SIZE];
    sprintf(buffer, "%s: %s", "todo yournick", cmd->chain[1]);

    for (int i = 0; i < MAX_CONN; i++) {
        if (clients[i] != 0 && i != cmd->nsd) {
            int n = send(clients[i], buffer, BUF_SIZE, 0);
            if (n < 0)
                perror("ERROR on responding");
        }
    }

    return "";
}
