#include "services.h"
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <stdlib.h>
#include "bcrypt.h"

static sqlite3 *db = NULL;

void services_init() {
    if (sqlite3_open("db/students.db", &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open DB: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    const char *sql = "CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, password TEXT);";
    char *err = NULL;
    if (sqlite3_exec(db, sql, 0, 0, &err) != SQLITE_OK) {
        fprintf(stderr, "DB error: %s\n", err);
        sqlite3_free(err);
        exit(1);
    }
}

int services_validate_user(const char *username, const char *password) {
/*    sqlite3_stmt *stmt;
    const char *sql = "SELECT password_hash FROM users WHERE username = ?;";
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
*/
    sqlite3_stmt *stmt;
char sql[256];
snprintf(sql, sizeof(sql), "SELECT password_hash FROM users WHERE username='%s';", username);
printf("[DEBUG] SQL: %s\n", sql);

if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    printf("[DEBUG] Prepare failed: %s\n", sqlite3_errmsg(db));
    return 0;
}

if (sqlite3_step(stmt) != SQLITE_ROW) {
    printf("[DEBUG] No user found for username: %s\n", username);
    sqlite3_finalize(stmt);
    return 0;
}

const unsigned char *hash = sqlite3_column_text(stmt, 0);
printf("[DEBUG] Stored hash: %s\n", hash);

int valid = (bcrypt_checkpw(password, (const char *)hash) == 0);
printf("[DEBUG] Password match: %d\n", valid);

sqlite3_finalize(stmt);
return valid;
}

/*
int services_create_user(const char *username, const char *password) {
    sqlite3_stmt *stmt;
    char hash[128];
    bcrypt_gensalt(12, hash);
    bcrypt_hashpw(password, hash, hash);
    printf("Creating user: %s\n", username);
    const char *sql = "INSERT INTO users (username, password) VALUES (?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) return 0;

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hash, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    printf("SQLite result code: %d\n", rc);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}
*/

int services_create_user(const char *username, const char *password) {
    char hash[128];
    bcrypt_gensalt(12, hash);
    bcrypt_hashpw(password, hash, hash);

    char *err = NULL;
    char sql[256];
    snprintf(sql, sizeof(sql),
             "INSERT INTO users(username, password_hash) VALUES('%s', '%s');",
             username, hash);

    int rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        printf("SQLite error: %s\n", err);
        return 0;
    }

    return 1;
}

int services_get_user(const char *username, User *user) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT username FROM users WHERE username = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) return 0;
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    int result = 0;
    if (rc == SQLITE_ROW) {
        strcpy(user->username, (const char *)sqlite3_column_text(stmt, 0));
        result = 1;
    }
    sqlite3_finalize(stmt);
    return result;
}
