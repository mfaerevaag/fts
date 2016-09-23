#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "engine.h"

int main(int argc, char *argv[])
{
    /* check args */
    if (argc < 3) {
        printf("usage: %s host port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *host = argv[1];
    int port = atoi(argv[2]);

    /* listen to interrupt ctrl-c */
    signal(SIGINT, stop);

    start(host, port);

    stop();
    return 0;
}
