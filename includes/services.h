// services.h

#ifndef SERVICES_H
#define SERVICES_H

#include <sqlite3.h>

typedef struct {
    int id;
    char username[64];
    char role[16];
    int is_active;
    char created_at[32];
} User;

// Initialize DB, returns 1 on success, 0 on failure
int services_init(const char *db_path);

// Close DB
void services_close(void);

// Create a user, returns 1 on success
int services_create_user(const char *username, const char *password);

// Validate user login, returns 1 if password matches
int services_validate_user(const char *username, const char *password);

// Get user info, returns 1 if found
int services_get_user(const char *username, User *out_user);

#endif // SERVICES_H
