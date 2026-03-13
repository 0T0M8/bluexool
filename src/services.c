// src/services.c
#include "services.h"
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include "bcrypt.h"

// Database pointer (open once)
static sqlite3 *db = NULL;

// -----------------------------
// Initialize DB
// -----------------------------
int services_init(const char *db_path)
{
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        printf("[services_init] Failed to open DB: %s\n", sqlite3_errmsg(db));
        return 0;
    }
    printf("[services_init] DB opened at %s\n", db_path);
    return 1;
}

// -----------------------------
// Create a new user
// -----------------------------
int services_create_user(const char *username, const char *password)
{
    if (!db) {
        printf("[services_create_user] DB not initialized\n");
        return 0;
    }

    char salt[64];
    char hash[128];

    // Generate bcrypt salt
    if (bcrypt_gensalt(12, salt) != 0) {
        printf("[services_create_user] Failed to generate salt\n");
        return 0;
    }

    // Hash password using the salt
    if (bcrypt_hashpw(password, salt, hash) != 0) {
        printf("[services_create_user] Failed to hash password\n");
        return 0;
    }

    const char *sql =
        "INSERT INTO users(username, password_hash) VALUES(?, ?);";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("[services_create_user] Prepare failed: %s\n",
               sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hash, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        printf("[services_create_user] Insert failed: %s\n",
               sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);

    printf("[services_create_user] User created: %s\n", username);

    return 1;
}

// -----------------------------
// Validate user login
// -----------------------------
int services_validate_user(const char *username, const char *password)
{
    if (!db) return 0;

    const char *sql = "SELECT password_hash FROM users WHERE username=?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("[services_validate_user] Prepare failed: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    int valid = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *hash = sqlite3_column_text(stmt, 0);
        printf("[services_validate_user] Stored hash: %s\n", hash);

        if (bcrypt_checkpw(password, (const char *)hash) == 0) {
            valid = 1;
        }
    } else {
        printf("[services_validate_user] No user found for: %s\n", username);
    }

    sqlite3_finalize(stmt);
    return valid;
}

// -----------------------------
// Close DB
// -----------------------------
void services_close(void)
{
    if (db) {
        sqlite3_close(db);
        db = NULL;
        printf("[services_close] DB closed\n");
    }
}

// Fetch user info by username
// Returns 1 if found, 0 if not
int services_get_user(const char *username, User *out_user)
{
    if (!db) return 0;

    const char *sql = "SELECT id, username, role, is_active, created_at FROM users WHERE username=?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("[services_get_user] Prepare failed: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out_user->id = sqlite3_column_int(stmt, 0);
        strncpy(out_user->username, (const char *)sqlite3_column_text(stmt, 1), sizeof(out_user->username)-1);
        strncpy(out_user->role, (const char *)sqlite3_column_text(stmt, 2), sizeof(out_user->role)-1);
        out_user->is_active = sqlite3_column_int(stmt, 3);
        strncpy(out_user->created_at, (const char *)sqlite3_column_text(stmt, 4), sizeof(out_user->created_at)-1);
        found = 1;
    } else {
        printf("[services_get_user] No user found for: %s\n", username);
    }

    sqlite3_finalize(stmt);
    return found;
}
