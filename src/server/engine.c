#include "engine.h"

void handle_new_conn();
void handle_data(int slot);

void sock_unblock(int sock);
void sock_listen();

int sd;
user **socks;
int highsock;
fd_set set_sockets;


void handle_new_conn()
{
    int nsd = accept(sd, NULL, NULL);
    if (nsd < 0) {
        perror("ERROR on accept");
    }

    sock_unblock(nsd);

    for (int i = 0; (i < MAX_CONN) && (nsd != -1); i ++) {
        if (socks[i] == NULL) {
            log_infof("slot %d connected\n", i);

            socks[i] = user_create(nsd);
            nsd = -1;
        }
    }

    if (nsd != -1) {
        char msg[] = "sorry, server full";
        log_warn("conn rejected, server full");

        int n = send(nsd, msg, ((int) strlen(msg) + 1), 0);
        if (n < 0) {
            log_err("failed to notify full server");
        }

        close(nsd);
    }
}

void handle_data(int slot)
{
    char buffer[BUF_SIZE];
    user *u = socks[slot];

    int n = recv(u->sock, buffer, BUF_SIZE, 0);
    if (n < 0) {
        log_warn("on receive, closing socket");
        close(u->sock);
        socks[slot] = 0;

    } else {
        /* remove trailing new line */
        buffer[strcspn(buffer, "\n")] = 0;

        log_infof("slot %d sent '%s'\n", slot, buffer);

        /* check if quit */
        if (strcmp(buffer, "/quit") == 0) {
            log_infof("slot %d quit\n", slot);
            close(u->sock);
            free(u);
            socks[slot] = NULL;
            return;
        }

        /* run command */
        cmd_handle(buffer, slot, socks);

        /* reply, if any */
        if (strcmp(buffer, "") != 0) {
            log_infof("replying '%s'\n", buffer);

            int n = send(u->sock, buffer, BUF_SIZE, 0);
            if (n < 0) {
                printf("ERROR: on responding\n");
            }
        }
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

void sock_listen()
{
    if (FD_ISSET(sd, &set_sockets))
        handle_new_conn();

    for (int i = 0; i < MAX_CONN; i++) {
        if (socks[i] != NULL)
            if (FD_ISSET(socks[i]->sock, &set_sockets))
                handle_data(i);
    }
}

void stop()
{
    log_info("stopping server...");

    char buffer[BUF_SIZE];
    sprintf(buffer, "/quit");

    for (int i = 0; i < MAX_CONN; i++) {
        if (socks[i] != NULL) {
            printf("FREE: slot %i ptr %p\n", i, (void *) socks[i]); // TODO
            int n = send(socks[i]->sock, buffer, strlen(buffer) + 1, 0);
            if (n < 0)
                perror("ERROR on quit");

            close(socks[i]->sock);
            free(socks[i]);
        }
    }

    free(socks);
    close(sd);
}

void start(int port)
{
    struct sockaddr_in server_address;
    struct timeval timeout;
    int nbr_sockets_lus;

    socks = malloc(MAX_CONN * sizeof(user *));

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

    log_info("listening...");
    listen(sd, MAX_CONN);

    while (1) {
        /* build list */
        FD_ZERO(&set_sockets);
        FD_SET(sd, &set_sockets);

        for (int i = 0; i < MAX_CONN; i++) {
            if (socks[i] != NULL) {
                FD_SET(socks[i]->sock, &set_sockets);
                if (socks[i]->sock > highsock)
                    highsock = socks[i]->sock;
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
