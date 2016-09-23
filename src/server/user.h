#ifndef USER_H
#define USER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct user_t {
    int sock;
    char *nick;
};
typedef struct user_t user;

user *user_create(int sock);

#endif
