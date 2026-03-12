#ifndef SERVICES_H
#define SERVICES_H

#include "http.h"

typedef struct {
    char username[64];
} User;

void services_init();

int services_validate_user(const char *username, const char *password);
int services_create_user(const char *username, const char *password);

// Fetch user info for dashboard (dummy example)
int services_get_user(const char *username, User *user);

#endif
