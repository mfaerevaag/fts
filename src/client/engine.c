#include "engine.h"

int sd;

pthread_t thread_listen;

void *sock_listen (void *sock)
{
    int n;
    char buffer[BUF_SIZE];

    while (1) {
        n = read(*(int *) sock, buffer, BUF_SIZE);
        if (n < 0) {
            perror("ERROR: on read");
            break;
        }

        if (n > 0){
            printf("\r%s\n", buffer);
            printf("\r%s", PROMPT);
            fflush(stdout);
        }

        if (strcmp(buffer, "/quit") == 0) {
            stop();
            break;
        }
    }

    pthread_exit(NULL);
    return NULL;
}


void stop()
{
    printf("stopping client...\n");

    if (sd != 0) {
        char buffer[BUF_SIZE];
        sprintf(buffer, "/quit");

        int n = send(sd, buffer, strlen(buffer) + 1, 0);
        if (n < 0)
            perror("ERROR on quit");

        close(sd);
    }

    exit(0);
}

void start(char *host, int port)
{
    char buffer[BUF_SIZE];
    struct sockaddr_in server;
    struct hostent* hp;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
        printf("ERROR: on create\n");
        return;
    }

    hp = gethostbyname(host);
    if (hp == 0) {
        printf("ERROR: get host\n");
        close(sd);
        return;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    memcpy((char *) &server.sin_addr, (char *) hp->h_addr, hp->h_length);

    if (connect(sd, (struct sockaddr *) &server, sizeof(struct sockaddr_in)) < 0) {
        perror("ERROR on connect");
        return;
    }

    printf("connected\n");

    pthread_create(&thread_listen, NULL, sock_listen, &sd);

    while (1) {
        printf("\r%s", PROMPT);
        fflush(stdout);

        memset(buffer, 0, BUF_SIZE);
        fgets(buffer, BUF_SIZE - 1, stdin);

        int n = send(sd, buffer, strlen(buffer) + 1, 0);
        if (n < 0)
            perror("ERROR on send");
    }

    close(sd);
}
