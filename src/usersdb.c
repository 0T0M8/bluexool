/* usersdb.c - production ready */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "usersdb.h"

/* ================= USERS ================= */

int db_create_user(sqlite3 *db,
    const char *username,
    const char *password_hash,
    const char *role)
{
    if (!db || !username || !password_hash)
        return -1;

    const char *sql =
        "INSERT INTO users (username, password_hash, role) "
        "VALUES (?, ?, ?)";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password_hash, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, role ? role : "user", -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE ? 0 : -1;
}

int db_get_user_by_username(sqlite3 *db,
    const char *username,
    int *user_id,
    char *password_hash_out,
    size_t hash_size,
    char *role_out,
    size_t role_size,
    int *is_active)
{
    if (!db || !username)
        return -1;

    const char *sql =
        "SELECT id, password_hash, role, is_active "
        "FROM users WHERE username = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        if (user_id)
            *user_id = sqlite3_column_int(stmt, 0);

        if (password_hash_out && hash_size > 0) {
            strncpy(password_hash_out,
                (const char *)sqlite3_column_text(stmt, 1),
                hash_size - 1);
            password_hash_out[hash_size - 1] = 0;
        }

        if (role_out && role_size > 0) {
            strncpy(role_out,
                (const char *)sqlite3_column_text(stmt, 2),
                role_size - 1);
            role_out[role_size - 1] = 0;
        }

        if (is_active)
            *is_active = sqlite3_column_int(stmt, 3);

        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);
    return -1;
}

int db_disable_user(sqlite3 *db, int user_id)
{
    const char *sql =
        "UPDATE users SET is_active = 0 WHERE id = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;

    sqlite3_bind_int(stmt, 1, user_id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE ? 0 : -1;
}

/* ================= SESSIONS ================= */

int db_create_session(sqlite3 *db,
    const char *session_id,
    int user_id,
    const char *expires_at)
{
    if (!db || !session_id || !expires_at)
        return -1;

    const char *sql =
        "INSERT INTO sessions (id, user_id, expires_at) "
        "VALUES (?, ?, ?)";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;

    sqlite3_bind_text(stmt, 1, session_id, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, user_id);
    sqlite3_bind_text(stmt, 3, expires_at, -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE ? 0 : -1;
}

int db_get_session(sqlite3 *db,
    const char *session_id,
    int *user_id_out,
    char *expires_at_out,
    size_t expires_size)
{
    if (!db || !session_id)
        return -1;

    const char *sql =
        "SELECT user_id, expires_at "
        "FROM sessions WHERE id = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;

    sqlite3_bind_text(stmt, 1, session_id, -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        if (user_id_out)
            *user_id_out = sqlite3_column_int(stmt, 0);

        if (expires_at_out && expires_size > 0) {
            strncpy(expires_at_out,
                (const char *)sqlite3_column_text(stmt, 1),
                expires_size - 1);
            expires_at_out[expires_size - 1] = 0;
        }

        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);
    return -1;
}

int db_delete_session(sqlite3 *db, const char *session_id)
{
    const char *sql =
        "DELETE FROM sessions WHERE id = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;

    sqlite3_bind_text(stmt, 1, session_id, -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE ? 0 : -1;
}

int db_cleanup_expired_sessions(sqlite3 *db, const char *now)
{
    const char *sql =
        "DELETE FROM sessions WHERE expires_at <= ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;

    sqlite3_bind_text(stmt, 1, now, -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE ? 0 : -1;
}
