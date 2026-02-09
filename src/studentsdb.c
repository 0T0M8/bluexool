/* studentsdb.c - production ready */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "studentsdb.h"

#define DB_PATH "db/students.db"

/* ================= INIT / CLOSE ================= */

int db_init(sqlite3 **db)
{
    if (!db) return -1;
    if (sqlite3_open(DB_PATH, db) != SQLITE_OK) {
        fprintf(stderr, "Failed to open DB: %s\n", sqlite3_errmsg(*db));
        return -1;
    }

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

    char *err = NULL;
    if (sqlite3_exec(*db, sql, NULL, NULL, &err) != SQLITE_OK) {
        fprintf(stderr, "Failed to create table: %s\n", err);
        sqlite3_free(err);
        sqlite3_close(*db);
        return -1;
    }

    return 0;
}

void db_close(sqlite3 *db)
{
    if (db) sqlite3_close(db);
}

/* ================= CRUD ================= */

int db_add_student(sqlite3 *db,
    const char *name, const char *gender, const char *dob,
    const char *status, const char *guardian, const char *contact1, const char *contact2)
{
    if (!db || !name || !gender || !dob || !status || !guardian || !contact1)
        return -1;

    const char *sql = "INSERT INTO students(name,gender,date_of_birth,status,guardian_name,contact1,contact2) VALUES(?,?,?,?,?,?,?)";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, gender, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, dob, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, status, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, guardian, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, contact1, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, contact2 ? contact2 : "", -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
        fprintf(stderr, "Insert failed: %s\n", sqlite3_errmsg(db));

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? 0 : -1;
}

/* ================= GET / SEARCH ================= */

static char *json_escape(const char *str) {
    if (!str) return strdup("");
    size_t len = strlen(str);
    size_t buf_size = len * 2 + 3; // rough estimate
    char *buf = malloc(buf_size);
    if (!buf) return NULL;

    char *p = buf;
    *p++ = '"';
    for (size_t i = 0; i < len; i++) {
        if (str[i] == '"' || str[i] == '\\') *p++ = '\\';
        *p++ = str[i];
    }
    *p++ = '"';
    *p = 0;
    return buf;
}

char *db_get_students(sqlite3 *db)
{
    if (!db) return strdup("[]");

    const char *sql = "SELECT id,name,gender,date_of_birth,status,guardian_name,contact1,contact2 FROM students";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return strdup("[]");
    }

    size_t buf_size = 8192;
    char *json = malloc(buf_size);
    if (!json) {
        sqlite3_finalize(stmt);
        return strdup("[]");
    }
    strcpy(json, "[");

    int first = 1;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) strcat(json, ",");
        first = 0;

        char *name = json_escape((const char *)sqlite3_column_text(stmt, 1));
        char *gender = json_escape((const char *)sqlite3_column_text(stmt, 2));
        char *dob = json_escape((const char *)sqlite3_column_text(stmt, 3));
        char *status = json_escape((const char *)sqlite3_column_text(stmt, 4));
        char *guardian = json_escape((const char *)sqlite3_column_text(stmt, 5));
        char *contact1 = json_escape((const char *)sqlite3_column_text(stmt, 6));
        char *contact2 = json_escape((const char *)sqlite3_column_text(stmt, 7));

        char row[1024];
        snprintf(row, sizeof(row),
                 "{\"id\":%d,\"name\":%s,\"gender\":%s,\"date_of_birth\":%s,\"status\":%s,\"guardian_name\":%s,\"contact1\":%s,\"contact2\":%s}",
                 sqlite3_column_int(stmt, 0),
                 name, gender, dob, status, guardian, contact1, contact2);

        strcat(json, row);

        free(name); free(gender); free(dob); free(status); free(guardian); free(contact1); free(contact2);
    }

    strcat(json, "]");
    sqlite3_finalize(stmt);
    return json;
}

char *db_search_students(sqlite3 *db, const char *field, const char *value)
{
    if (!db || !field || !value) return strdup("[]");

    char sql[512];
    snprintf(sql, sizeof(sql),
             "SELECT id,name,gender,date_of_birth,status,guardian_name,contact1,contact2 "
             "FROM students WHERE %s LIKE ?", field);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return strdup("[]");
    }

    char query[256];
    snprintf(query, sizeof(query), "%%%s%%", value);
    sqlite3_bind_text(stmt, 1, query, -1, SQLITE_TRANSIENT);

    size_t buf_size = 8192;
    char *json = malloc(buf_size);
    if (!json) {
        sqlite3_finalize(stmt);
        return strdup("[]");
    }
    strcpy(json, "[");

    int first = 1;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) strcat(json, ",");
        first = 0;

        char *name = json_escape((const char *)sqlite3_column_text(stmt, 1));
        char *gender = json_escape((const char *)sqlite3_column_text(stmt, 2));
        char *dob = json_escape((const char *)sqlite3_column_text(stmt, 3));
        char *status = json_escape((const char *)sqlite3_column_text(stmt, 4));
        char *guardian = json_escape((const char *)sqlite3_column_text(stmt, 5));
        char *contact1 = json_escape((const char *)sqlite3_column_text(stmt, 6));
        char *contact2 = json_escape((const char *)sqlite3_column_text(stmt, 7));

        char row[1024];
        snprintf(row, sizeof(row),
                 "{\"id\":%d,\"name\":%s,\"gender\":%s,\"date_of_birth\":%s,\"status\":%s,\"guardian_name\":%s,\"contact1\":%s,\"contact2\":%s}",
                 sqlite3_column_int(stmt, 0),
                 name, gender, dob, status, guardian, contact1, contact2);

        strcat(json, row);

        free(name); free(gender); free(dob); free(status); free(guardian); free(contact1); free(contact2);
    }

    strcat(json, "]");
    sqlite3_finalize(stmt);
    return json;
}

/* ================= UPDATE / DELETE ================= */

int db_update_student(sqlite3 *db, int id,
    const char *name, const char *gender, const char *dob,
    const char *status, const char *guardian, const char *contact1, const char *contact2)
{
    if (!db || id <= 0) return -1;

    const char *sql =
        "UPDATE students SET name=?, gender=?, date_of_birth=?, status=?, guardian_name=?, contact1=?, contact2=? WHERE id=?";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, gender, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, dob, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, status, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, guardian, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, contact1, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, contact2 ? contact2 : "", -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 8, id);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) fprintf(stderr, "Update failed: %s\n", sqlite3_errmsg(db));

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? 0 : -1;
}

int db_delete_student(sqlite3 *db, int id)
{
    if (!db || id <= 0) return -1;

    const char *sql = "DELETE FROM students WHERE id=?";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) fprintf(stderr, "Delete failed: %s\n", sqlite3_errmsg(db));

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? 0 : -1;
}
