#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <ctype.h>
#include <signal.h>

#define MAX_CONN 5
#define BUF_SIZE 512

struct commande {
    char**  chain;
    int     nbr_token;
    int     nsd;
};

struct commande cmd;
int        sd;
int        conn_list[MAX_CONN];
fd_set     set_sockets;
int        highsock;


void decode_command(char buffer[BUF_SIZE]);
void sock_unblock(int sock);
void build_select_list();
void handle_new_conn();
void handle_data(int pos);
void sock_listen();
void server_start();
void server_stop();


/* decode command from client */
void decode_command(char buffer[BUF_SIZE])
{
    char copy[BUF_SIZE];
    sprintf(copy, "%s", buffer);

    cmd.chain = (char **) malloc(1 * sizeof(char *));

    int nbr_token = 0;
    int l = 0;
    int pos = 0;

    char *pch;
    pch = strtok(copy, " ");

    while (pch != NULL) {
        nbr_token++;
        cmd.chain = (char **) realloc(cmd.chain, (nbr_token * sizeof(char *)));

        if (nbr_token < 3) {
            l = (int) strlen(pch);
            pos = pos + l + 1;

            cmd.chain[nbr_token - 1] = (char *) malloc((l + 1) * sizeof(char));
            sprintf(cmd.chain[nbr_token - 1], "%s", pch);

        } else {
            cmd.chain[nbr_token - 1] = (char *) malloc(((int) strlen(&buffer[pos]) + 1) * sizeof(char));
            sprintf(cmd.chain[nbr_token - 1], "%s", &buffer[pos]);
            break;
        }

        pch = strtok(NULL, " ");
    }

    cmd.nbr_token = nbr_token;
}

/* set socket as non blocking */
void sock_unblock(int sock)
{
    int opts = fcntl(sock, F_GETFL);

    if (opts < 0) {
        perror("ERROR: fcntl(F_GETFL)");
    } else {
        opts = (opts | O_NONBLOCK);
        if (fcntl(sock,F_SETFL,opts) < 0)
            perror("ERROR: fcntl(F_SETFL)");
    }
}

/* add sock num to connection list */
void build_select_list()
{
    FD_ZERO(&set_sockets);
    FD_SET(sd, &set_sockets);

    for (int i = 0; i < MAX_CONN; i++) {
        if (conn_list[i] != 0) {
            FD_SET(conn_list[i], &set_sockets);
            if (conn_list[i] > highsock)
                highsock = conn_list[i];
        }
    }
}

/* handle connection */
void handle_new_conn()
{
    int nsd = accept(sd, NULL, NULL);
    if (nsd < 0) {
        perror("ERROR: on accepting socket");
    }

    sock_unblock(nsd);

    for (int i = 0; (i < MAX_CONN) && (nsd != -1); i ++) {
        if (conn_list[i] == 0) {
            printf("INFO: accepted connection fd=%d; slot=%d\n", nsd, i);
            conn_list[i] = nsd;
            nsd = -1;
        }
    }

    /* server full */
    if (nsd != -1) {
        char err[] = "sorry, server's full";
        printf("ERROR: %s\n", err);

        int n = send(nsd, err, ((int) strlen(err) + 1), 0);
        if (n < 0) {
            printf("ERROR: cannot notify server's full\n");
        }

        close(nsd);
    }
}

/* handle data from client */
void handle_data(int pos)
{
    int n;
    char buffer[BUF_SIZE];
    int nsd = conn_list[pos];

    printf("INFO: handling data fd=%d pos=%d\n", nsd, pos);

    n = recv(nsd, buffer, BUF_SIZE, 0);
    if (n < 0) {
        printf("ERROR: could not receive\n");
        close(nsd);
        conn_list[pos] = 0;

    } else {
        printf("INFO: chain received: %s\n", buffer);

        decode_command(buffer);
        cmd.nsd = nsd;

        printf("command: %s\n", cmd.chain[0]);

        if (strcmp(cmd.chain[0], "/nick") == 0) {
            /* sprintf(buffer, "%s", slash_nick()); */
            sprintf(buffer, "TODO: slash_nick()");

        } else if (strcmp(cmd.chain[0], "/info") == 0) {
            /* sprintf(buffer, "%s", slash_info()); */
            sprintf(buffer, "TODO: slash_info()");

        } else if (strcmp(cmd.chain[0], "/quit") == 0) {
            /* sprintf(buffer, "%s", slash_quitter(pos) ); */
            sprintf(buffer, "TODO: slash_quit(pos)");

        } else {
            sprintf(buffer, "WARNING: unknown command '%s'", cmd.chain[0]);
        }

        /* unless buffer is empty */
        if (strcmp(buffer, "<EMPTY>") == 1) {
            n = send(nsd, buffer, BUF_SIZE, 0);
            if (n < 0) {
                printf("ERROR: error on responding\n");
            }

            printf("REPONSE: %s\n", buffer);
        }

        /* in command and buffer is quit */
        if (strcmp(cmd.chain[0], "/quit") == 0 &&
            strcmp(buffer, "<QUIT>") == 0) {
            close(cmd.nsd);
        }
    }
}

/* handle data on sockets */
void sock_listen()
{
    if (FD_ISSET(sd, &set_sockets))
        handle_new_conn();

    for (int i = 0; i < MAX_CONN; i++) {
        if (FD_ISSET(conn_list[i], &set_sockets))
            handle_data(i);
    }
}

/* start server loop */
void server_start(int port)
{
    struct sockaddr_in server_address;
    struct timeval timeout;
    int nbr_sockets_lus;


    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
        perror("ERROR: socket\n");
        exit(EXIT_FAILURE);
    }


    int reuse_addr = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));


    sock_unblock(sd);


    server_address.sin_family      = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port        = htons(port);

    if (bind(sd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0 ) {
        perror("ERROR: on bind\n");
        close(sd);
        exit(EXIT_FAILURE);
    }


    highsock = sd;
    memset((char *) &conn_list, 0, sizeof(conn_list));


    printf("listening to port %i...\n", port);
    listen(sd, MAX_CONN);


    while (1) {
        build_select_list();
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        nbr_sockets_lus = select(highsock + 1, &set_sockets, NULL, NULL, &timeout);

        if (nbr_sockets_lus < 0) {
            perror("ERROR: couldn't select socket\n");
            break;

        } else if (nbr_sockets_lus == 0) {
            /* printf("Rien a lire. Serveur en vie...\n"); */
            fflush(stdout);
        } else {
            sock_listen();
        }
    }
}

/* stop server loop */
void server_stop()
{
    printf("stoping server...\n");

    for (int i = 0; i < MAX_CONN; i++) {
        if (conn_list[i] != 0)
            close(conn_list[i]);
    }

    close(sd);
}

int main(int argc, char* argv[])
{
    /* check args */
    if (argc < 2) {
        printf("usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    /* listen to ctrl-c */
    signal(SIGINT, server_stop);

    server_start(port);

    server_stop();
    return 0;
}
