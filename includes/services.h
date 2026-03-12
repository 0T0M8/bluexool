#ifndef SERVICES_H
#define SERVICES_H

#include "http.h"

// Example user structure
typedef struct {
    char username[64];
    char password[64]; // hashed password
} User;

// Initialize the database / services
void services_init();

// User-related services
int services_validate_user(const char *username, const char *password);
int services_create_user(const char *username, const char *password);

#endif
