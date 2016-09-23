#include "user.h"

user *user_create(int sock)
{
    user *u = malloc(sizeof(user));

    u->sock = sock;

    char *nick = "anon";
    u->nick = calloc(strlen(nick), sizeof(char));
    memcpy(u->nick, nick, strlen(nick));

    return u;
}
