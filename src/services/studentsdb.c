/* studentsdb.c - production ready */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "services/studentsdb.h"

#define DB_PATH "db/students.db"
#define DB_SCHEMA_VERSION 2

/* ================= INTERNAL HELPERS ================= */

static int db_exec(sqlite3 *db, const char *sql)
{
    char *err = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err);
        sqlite3_free(err);
        return -1;
    }
    return 0;
}

static int db_get_user_version(sqlite3 *db)
{
    sqlite3_stmt *stmt;
    int version = 0;

    if (sqlite3_prepare_v2(db, "PRAGMA user_version;", -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW)
            version = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return version;
}

static int db_set_user_version(sqlite3 *db, int version)
{
    char sql[64];
    snprintf(sql, sizeof(sql), "PRAGMA user_version = %d;", version);
    return db_exec(db, sql);
}

/* ================= MIGRATIONS ================= */

/* v1: students table */
static int migrate_v1(sqlite3 *db)
{
    const char *sql =
        "CREATE TABLE IF NOT EXISTS students ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "gender TEXT NOT NULL,"
        "date_of_birth TEXT NOT NULL,"
        "status TEXT NOT NULL,"
        "guardian_name TEXT NOT NULL,"
        "contact1 TEXT NOT NULL,"
        "contact2 TEXT"
        ");";

    return db_exec(db, sql);
}

/* v2: users + sessions */
static int migrate_v2(sqlite3 *db)
{
    const char *sql =
        "PRAGMA foreign_keys = ON;"

        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL COLLATE NOCASE UNIQUE,"
        "password_hash TEXT NOT NULL,"
        "role TEXT NOT NULL DEFAULT 'user',"
        "is_active INTEGER NOT NULL DEFAULT 1,"
        "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"
        ");"

        "CREATE TABLE IF NOT EXISTS sessions ("
        "id TEXT PRIMARY KEY,"
        "user_id INTEGER NOT NULL,"
        "expires_at TEXT NOT NULL,"
        "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE"
        ");"

        "CREATE INDEX IF NOT EXISTS idx_sessions_user "
        "ON sessions(user_id);";

    return db_exec(db, sql);
}

/* ================= INIT / CLOSE ================= */

int db_init(sqlite3 **db)
{
    if (sqlite3_open(DB_PATH, db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database\n");
        return -1;
    }

    int version = db_get_user_version(*db);

    if (version < 1) {
        if (migrate_v1(*db) < 0) return -1;
        if (db_set_user_version(*db, 1) < 0) return -1;
        version = 1;
    }

    if (version < 2) {
        if (migrate_v2(*db) < 0) return -1;
        if (db_set_user_version(*db, 2) < 0) return -1;
        version = 2;
    }

    return 0;
}

void db_close(sqlite3 *db)
{
    if (db) sqlite3_close(db);
}

/* ================= STUDENTS CRUD ================= */

int db_add_student(sqlite3 *db,
    const char *name, const char *gender, const char *dob,
    const char *status, const char *guardian,
    const char *contact1, const char *contact2)
{
    if (!db || !name || !gender || !dob || !status || !guardian || !contact1)
        return -1;

    const char *sql =
        "INSERT INTO students "
        "(name,gender,date_of_birth,status,guardian_name,contact1,contact2) "
        "VALUES(?,?,?,?,?,?,?)";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, gender, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, dob, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, status, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, guardian, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, contact1, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, contact2 ? contact2 : "", -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? 0 : -1;
}

/* ================= JSON HELPERS ================= */

static char *json_escape(const char *s)
{
    if (!s) return strdup("\"\"");
    size_t len = strlen(s);
    char *out = malloc(len * 2 + 3);
    char *p = out;
    *p++ = '"';

    for (; *s; s++) {
        if (*s == '"' || *s == '\\') *p++ = '\\';
        *p++ = *s;
    }

    *p++ = '"';
    *p = 0;
    return out;
}

/* ================= GET / SEARCH ================= */

static char *students_to_json(sqlite3_stmt *stmt)
{
    size_t size = 8192;
    char *json = malloc(size);
    strcpy(json, "[");

    int first = 1;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) strcat(json, ",");
        first = 0;

        char *name = json_escape((char *)sqlite3_column_text(stmt, 1));
        char *gender = json_escape((char *)sqlite3_column_text(stmt, 2));
        char *dob = json_escape((char *)sqlite3_column_text(stmt, 3));
        char *status = json_escape((char *)sqlite3_column_text(stmt, 4));
        char *guardian = json_escape((char *)sqlite3_column_text(stmt, 5));
        char *contact1 = json_escape((char *)sqlite3_column_text(stmt, 6));
        char *contact2 = json_escape((char *)sqlite3_column_text(stmt, 7));

        char row[1024];
        snprintf(row, sizeof(row),
            "{\"id\":%d,\"name\":%s,\"gender\":%s,\"date_of_birth\":%s,"
            "\"status\":%s,\"guardian_name\":%s,\"contact1\":%s,\"contact2\":%s}",
            sqlite3_column_int(stmt, 0),
            name, gender, dob, status, guardian, contact1, contact2);

        strcat(json, row);

        free(name); free(gender); free(dob);
        free(status); free(guardian);
        free(contact1); free(contact2);
    }

    strcat(json, "]");
    return json;
}

char *db_get_students(sqlite3 *db)
{
    const char *sql =
        "SELECT id,name,gender,date_of_birth,status,guardian_name,contact1,contact2 "
        "FROM students";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return strdup("[]");

    char *json = students_to_json(stmt);
    sqlite3_finalize(stmt);
    return json;
}

char *db_search_students(sqlite3 *db, const char *field, const char *value)
{
    if (!db || !field || !value) return strdup("[]");

    char sql[256];
    snprintf(sql, sizeof(sql),
        "SELECT id,name,gender,date_of_birth,status,guardian_name,contact1,contact2 "
        "FROM students WHERE %s LIKE ?", field);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return strdup("[]");

    char q[256];
    snprintf(q, sizeof(q), "%%%s%%", value);
    sqlite3_bind_text(stmt, 1, q, -1, SQLITE_TRANSIENT);

    char *json = students_to_json(stmt);
    sqlite3_finalize(stmt);
    return json;
}

/* ================= UPDATE / DELETE ================= */

int db_update_student(sqlite3 *db, int id,
    const char *name, const char *gender, const char *dob,
    const char *status, const char *guardian,
    const char *contact1, const char *contact2)
{
    const char *sql =
        "UPDATE students SET "
        "name=?, gender=?, date_of_birth=?, status=?, guardian_name=?, "
        "contact1=?, contact2=? WHERE id=?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, gender, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, dob, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, status, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, guardian, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, contact1, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, contact2 ? contact2 : "", -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 8, id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? 0 : -1;
}

int db_delete_student(sqlite3 *db, int id)
{
    const char *sql = "DELETE FROM students WHERE id=?";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;

    sqlite3_bind_int(stmt, 1, id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? 0 : -1;
}
