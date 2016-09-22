#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "engine.h"

int main(int argc, char *argv[])
{
    /* check args */
    if (argc < 2) {
        printf("usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    /* listen to interrupt ctrl-c */
    signal(SIGINT, stop);

    start(port);

    stop();
    return 0;
}
