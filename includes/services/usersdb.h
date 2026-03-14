#ifndef USERSDB_H
#define USERSDB_H

#include <stddef.h>
#include <sqlite3.h>

/* ================= USERS ================= */

/*
 * Create a new user.
 * - username: unique, case-insensitive
 * - password_hash: bcrypt/argon2 encoded string
 * - role: NULL defaults to "user"
 *
 * Returns 0 on success, -1 on failure.
 */
int db_create_user(sqlite3 *db,
    const char *username,
    const char *password_hash,
    const char *role);

/*
 * Fetch user credentials by username.
 *
 * Outputs:
 * - user_id (optional)
 * - password_hash_out (optional buffer)
 * - role_out (optional buffer)
 * - is_active (optional)
 *
 * Returns 0 if user found, -1 otherwise.
 */
int db_get_user_by_username(sqlite3 *db,
    const char *username,
    int *user_id,
    char *password_hash_out,
    size_t hash_size,
    char *role_out,
    size_t role_size,
    int *is_active);

/*
 * Disable (soft-delete) a user account.
 * Sets is_active = 0.
 *
 * Returns 0 on success, -1 on failure.
 */
int db_disable_user(sqlite3 *db, int user_id);

/* ================= SESSIONS ================= */

/*
 * Create a session for a user.
 * - session_id must be cryptographically random
 * - expires_at should be ISO-8601 UTC timestamp
 *
 * Returns 0 on success, -1 on failure.
 */
int db_create_session(sqlite3 *db,
    const char *session_id,
    int user_id,
    const char *expires_at);

/*
 * Fetch a session by session_id.
 *
 * Outputs:
 * - user_id_out (optional)
 * - expires_at_out (optional buffer)
 *
 * Returns 0 if session exists, -1 otherwise.
 */
int db_get_session(sqlite3 *db,
    const char *session_id,
    int *user_id_out,
    char *expires_at_out,
    size_t expires_size);

/*
 * Delete a session (logout).
 *
 * Returns 0 on success, -1 on failure.
 */
int db_delete_session(sqlite3 *db, const char *session_id);

/*
 * Remove all expired sessions.
 * - now: current UTC time in ISO-8601 format
 *
 * Returns 0 on success, -1 on failure.
 */
int db_cleanup_expired_sessions(sqlite3 *db, const char *now);

#endif /* USERSDB_H */
