#include "engine.h"

void handle_new_conn();
void handle_data(int pos);

void sock_unblock(int sock);
void sock_listen();

int sd;
int socks[MAX_CONN];
fd_set set_sockets;
int highsock;


void handle_new_conn()
{
    int nsd = accept(sd, NULL, NULL);
    if (nsd < 0) {
        perror("ERROR on accept");
    }

    sock_unblock(nsd);

    int i, n;
    for (i = 0; (i < MAX_CONN) && (nsd != -1); i ++)
        if (socks[i] == 0) {
            log_infof("slot %d connected\n", i);
            socks[i] = nsd;
            nsd = -1;
        }

    if (nsd != -1) {
        char msg[] = "sorry, server full";
        log_warn("conn rejected, server full");

        n = send(nsd, msg, ((int) strlen(msg) + 1), 0);
        if (n < 0) {
            log_err("failed to notify full server");
        }

        close(nsd);
    }
}

void handle_data(int slot)
{
    char buffer[BUF_SIZE];
    int nsd = socks[slot];

    int n = recv(nsd, buffer, BUF_SIZE, 0);
    if (n < 0) {
        log_warn("on receive, closing socket");
        close(nsd);
        socks[slot] = 0;

    } else {
        /* remove trailing new line */
        buffer[strcspn(buffer, "\n")] = 0;

        log_infof("slot %d sent '%s'\n", slot, buffer);

        /* TODO return */
        cmd_handle(buffer, slot, socks);

        /* sprintf(buffer, "todo"); */
        /* int n = send(nsd, buffer, BUF_SIZE, 0); */
        /* if (n < 0) { */
        /*     printf("ERROR: on responding\n"); */
        /* } */

        /* /\* unless buffer is empty *\/ */
        /* if (strcmp(buffer, "<EMPTY>") != 0) { */
        /*     n = send(nsd, buffer, BUF_SIZE, 0); */
        /*     if (n < 0) { */
        /*         printf("ERROR: on responding\n"); */
        /*     } */
        /* } */

        /* if (strcmp(cmd->chain[0], "/quit") == 0 && */
        /*     strcmp(buffer, "<QUIT>") == 0) */
        /*     close(cmd->nsd); */
    }
}

void sock_unblock(int sock)
{
    int opts = fcntl(sock,F_GETFL);

    if (opts < 0) {
        perror("ERROR fcntl(F_GETFL)");

    } else {
        opts = (opts | O_NONBLOCK);
        if (fcntl(sock,F_SETFL,opts) < 0)
            perror("ERROR fcntl(F_SETFL)");
    }
}

void sock_listen() {
    if (FD_ISSET(sd, &set_sockets))
        handle_new_conn();

    for (int i = 0; i < MAX_CONN; i++) {
        if (FD_ISSET(socks[i], &set_sockets))
            handle_data(i);
    }
}

void stop()
{
    log_info("stopping server...");

    char buffer[BUF_SIZE];
    sprintf(buffer, "/quit");

    for (int i = 0; i < MAX_CONN; i++) {
        if (socks[i] != 0) {
            int n = send(socks[i], buffer, strlen(buffer) + 1, 0);
            if (n < 0)
                perror("ERROR on quit");

            close(socks[i]);
        }
    }

    close(sd);
}

void start(int port)
{
    struct sockaddr_in server_address;
    struct timeval timeout;
    int nbr_sockets_lus;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
        perror("ERROR socket");
        exit(EXIT_FAILURE);
    }

    int reuse_addr = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));

    sock_unblock(sd);

    server_address.sin_family      = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port        = htons(port);

    if (bind(sd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("ERROR on bind");
        close(sd);
        exit(EXIT_FAILURE);
    }

    highsock = sd;
    memset((char *) &socks, 0, sizeof(socks));

    log_info("listening...");
    listen(sd, MAX_CONN);

    while (1) {
        /* build list */
        FD_ZERO(&set_sockets);
        FD_SET(sd, &set_sockets);

        for (int i = 0; i < MAX_CONN; i++) {
            if (socks[i] != 0) {
                FD_SET(socks[i], &set_sockets);
                if (socks[i] > highsock)
                    highsock = socks[i];
            }
        }

        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        nbr_sockets_lus = select(highsock + 1, &set_sockets, NULL, NULL, &timeout);

        if (nbr_sockets_lus < 0) {
            perror("ERROR on select");
            break;
        } else if (nbr_sockets_lus == 0) {
            fflush(stdout);
        } else {
            sock_listen();
        }
    }
}
