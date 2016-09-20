#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MSGNR 0
void msg_handler(int,int);

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int cntr = MSGNR;

    if (argc < 2) {
        fprintf(stderr,"usage: %s port\n", argv[0]);
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    printf("Waiting for connections\n");

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while (1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0)
            error("ERROR on accept");

        pid = fork();
        if (pid < 0)
            error("ERROR on fork");

        if (pid == 0)  {
            close(sockfd);
            cntr++;
            msg_handler(newsockfd,cntr);
            exit(0);
        }
        else {
            close(newsockfd);
        }
    }

    close(newsockfd);
    close(sockfd);
    return 0;
}

void msg_handler(int sock, int msg_nr)
{
    int n;
    char buffer[256];

    bzero(buffer, 256);
    n = read(sock, buffer, 255);

    if (n < 0) error("ERROR reading from socket");

    buffer[strcspn(buffer, "\n")] = 0; /* remove trailing \n */
    printf("%i[%i]: %s\n", sock, msg_nr, buffer);

    n = write(sock, "I got your message", 18);

    if (n < 0) error("ERROR writing to socket");
}
