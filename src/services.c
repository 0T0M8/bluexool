#include "services.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bcrypt.h"  // your existing bcrypt wrapper
#include <sqlite3.h>

static sqlite3 *db = NULL;

void services_init() {
    if (sqlite3_open("db/students.db", &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open DB: %s\n", sqlite3_errmsg(db));
        exit(EXIT_FAILURE);
    }

    // Example: create users table if not exists
    const char *sql = "CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, password TEXT);";
    char *err = NULL;
    if (sqlite3_exec(db, sql, 0, 0, &err) != SQLITE_OK) {
        fprintf(stderr, "DB error: %s\n", err);
        sqlite3_free(err);
        exit(EXIT_FAILURE);
    }
}

// Return 1 if valid user, 0 otherwise
int services_validate_user(const char *username, const char *password) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT password FROM users WHERE username = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) return 0;
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    int result = 0;

    if (rc == SQLITE_ROW) {
        const char *hash = (const char *)sqlite3_column_text(stmt, 0);
        if (bcrypt_checkpw(password, hash) == 0) result = 1;
    }

    sqlite3_finalize(stmt);
    return result;
}

// Create user, returns 1 on success
int services_create_user(const char *username, const char *password) {
    sqlite3_stmt *stmt;
    char hash[128];
    bcrypt_gensalt(12, hash); // generate salt & hash
    bcrypt_hashpw(password, hash, hash);

    const char *sql = "INSERT INTO users (username, password) VALUES (?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) return 0;

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hash, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 1 : 0;
}
