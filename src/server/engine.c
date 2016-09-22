#include "engine.h"

void handle_new_conn();
void handle_data(int pos);

void sock_unblock(int sock);
void sock_listen();

int sd;
int conn_list[MAX_CONN];
fd_set set_sockets;
int highsock;


void handle_new_conn()
{
    int nsd = accept(sd, NULL, NULL);
    if (nsd < 0) {
        perror("ERROR: on accept");
    }

    sock_unblock(nsd);

    int i, n;
    for (i = 0; (i < MAX_CONN) && (nsd != -1); i ++)
        if (conn_list[i] == 0) {
            printf("INFO: accepted connection fd=%d slot=%d\n", nsd, i);
            conn_list[i] = nsd;
            nsd = -1;
        }

    if (nsd != -1) {
        char msg[] = "sorry, server full";
        printf("ERROR: %s\n", msg);

        n = send(nsd, msg, ((int) strlen(msg) + 1), 0);
        if (n < 0) {
            printf("ERROR: failed to notify full server\n");
        }

        close(nsd);
    }
}

void handle_data(int pos)
{
    char buffer[BUF_SIZE];
    int n;
    int nsd = conn_list[pos];

    printf("INFO: handling data fd=%d pos=%d\n", nsd, pos);

    n = recv(nsd, buffer, BUF_SIZE, 0);
    if (n < 0) {
        printf("ERROR: on receive\n");
        close(nsd);
        conn_list[pos] = 0;

    } else {
        /* remove trailing new line */
        buffer[strcspn(buffer, "\n")] = 0;

        printf("INFO: chain received '%s'\n", buffer);

        command *cmd = cmd_decode(buffer);
        cmd->nsd = nsd;

        printf("INFO: cmd '%s'\n", cmd->chain[0]);

        sprintf(buffer, "%s", cmd_handle(cmd));

        /* unless buffer is empty */
        if (strcmp(buffer, "<EMPTY>") != 0) {
            n = send(nsd, buffer, BUF_SIZE, 0);
            if (n < 0) {
                printf("ERROR: on responding\n");
            }
            printf("RESPONSE: %s\n", buffer);
        }

        if (strcmp(cmd->chain[0], "/quit") == 0 &&
            strcmp(buffer, "<QUIT>") == 0)
            close(cmd->nsd);

        free(cmd);
    }
}

void sock_unblock(int sock)
{
    int opts = fcntl(sock,F_GETFL);

    if (opts < 0) {
        perror("ERROR: fcntl(F_GETFL)");

    } else {
        opts = (opts | O_NONBLOCK);
        if (fcntl(sock,F_SETFL,opts) < 0)
            perror("ERROR: fcntl(F_SETFL)");
    }
}

void sock_listen() {
    if (FD_ISSET(sd, &set_sockets))
        handle_new_conn();

    int i;
    for (i = 0; i < MAX_CONN; i++) {
        if (FD_ISSET(conn_list[i], &set_sockets))
            handle_data(i);
    }
}

void stop()
{
    printf("stoping server...\n");

    int i;
    for (i = 0; i < MAX_CONN; i++) {
        if (conn_list[i] != 0)
            close(conn_list[i]);
    }

    close (sd);
}

void start(int port)
{
    struct sockaddr_in server_address;
    struct timeval timeout;
    int nbr_sockets_lus;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
        perror("ERROR: socket");
        exit(EXIT_FAILURE);
    }

    int reuse_addr = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));

    sock_unblock (sd);

    server_address.sin_family      = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port        = htons(port);

    if (bind(sd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("ERROR: on bind\n");
        close(sd);
        exit(EXIT_FAILURE);
    }

    highsock = sd;
    memset((char *) &conn_list, 0, sizeof(conn_list));

    printf("listening...\n");
    listen(sd, MAX_CONN);

    while (1) {
        /* build list */
        FD_ZERO(&set_sockets);
        FD_SET(sd, &set_sockets);

        int i;
        for (i = 0; i < MAX_CONN; i++) {
            if (conn_list[i] != 0) {
                FD_SET(conn_list[i], &set_sockets);
                if (conn_list[i] > highsock)
                    highsock = conn_list[i];
            }
        }

        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        nbr_sockets_lus = select(highsock+1, &set_sockets, NULL, NULL, &timeout);

        if (nbr_sockets_lus < 0) {
            perror("ERROR: on select");
            break;

        } else if (nbr_sockets_lus == 0) {
            fflush(stdout);
        } else {
            sock_listen();
        }
    }
}
