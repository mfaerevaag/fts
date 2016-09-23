#include "command.h"

char *cmd_nick(command *cmd, user **socks);
char *cmd_broadcast(command *cmd, user **socks);

command *cmd_decode(command *cmd, char buffer[BUF_SIZE])
{
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

void cmd_handle(char buffer[BUF_SIZE], int slot, user **socks)
{
    /* decode */
    command cmd;
    cmd_decode(&cmd, buffer);
    cmd.slot = slot;

    user *u = socks[slot];

    memset(buffer, 0, BUF_SIZE);

    if (cmd.chain[0] != NULL) {
        if (strcmp(cmd.chain[0], "/nick") == 0) {
            sprintf(buffer, "%s", cmd_nick(&cmd, socks));

        } else if (strcmp(cmd.chain[0], "/all") == 0) {
            sprintf(buffer, "%s", cmd_broadcast(&cmd, socks));

        } else if (strcmp(cmd.chain[0], "/quit") == 0) {
            log_infof("slot %d quit\n", slot);
            free(socks[slot]);

        } else {
            log_warn("unknown command");
            sprintf(buffer, "unknown command");
        }

        int n = send(u->sock, buffer, BUF_SIZE, 0);
        if (n < 0)
            perror("ERROR on responding");
    }
}

char *cmd_nick(command *cmd, user **socks)
{
    /* check args */
    if (cmd->len != 2) {
        return "usage: /nick <name>";
    }

    user *u = socks[cmd->slot];

    u->nick = cmd->chain[1];

    return "changed nick";
}

char *cmd_broadcast(command *cmd, user **socks)
{
    /* check args */
    if (cmd->len < 2) {
        return "usage: /all <msg>";
    }

    user *u = socks[cmd->slot];

    char buffer[BUF_SIZE];
    sprintf(buffer, "%s: ", u->nick);
    for (int i = 1; i < cmd->len; i++) {
        sprintf(buffer, "%s %s", buffer, cmd->chain[i]);
    }

    for (int i = 0; i < MAX_CONN; i++) {
        if (socks[i] != NULL && i != cmd->slot) {
            int n = send(socks[i]->sock, buffer, BUF_SIZE, 0);
            if (n < 0)
                perror("ERROR on responding");
        }
    }

    return "";
}
