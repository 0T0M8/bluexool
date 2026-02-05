#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "studentsdb.h"

#define DB_PATH "db/students.db"

int db_init(sqlite3 **db)
{
    if (sqlite3_open(DB_PATH, db) != SQLITE_OK)
        return -1;

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
        sqlite3_free(err);
        return -1;
    }

    return 0;
}

void db_close(sqlite3 *db)
{
    sqlite3_close(db);
}

/* ================= CRUD ================= */

int db_add_student(sqlite3 *db,
    const char *name, const char *gender, const char *dob,
    const char *status, const char *guardian, const char *contact1, const char *contact2)
{
    const char *sql = "INSERT INTO students(name,gender,date_of_birth,status,guardian_name,contact1,contact2) VALUES(?,?,?,?,?,?,?)";
    sqlite3_stmt *stmt;

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, gender, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, dob, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, status, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, guardian, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, contact1, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, contact2 ? contact2 : "", -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? 0 : -1;
}

char *db_get_students(sqlite3 *db)
{
    const char *sql = "SELECT id,name,gender,date_of_birth,status,guardian_name,contact1,contact2 FROM students";
    sqlite3_stmt *stmt;

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    char *json = malloc(8192); // increased buffer for more fields
    strcpy(json, "[");

    int first = 1;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) strcat(json, ",");
        first = 0;

        char row[512];
        snprintf(row, sizeof(row),
            "{\"id\":%d,\"name\":\"%s\",\"gender\":\"%s\",\"date_of_birth\":\"%s\","
            "\"status\":\"%s\",\"guardian_name\":\"%s\",\"contact1\":\"%s\",\"contact2\":\"%s\"}",
            sqlite3_column_int(stmt, 0),
            sqlite3_column_text(stmt, 1),
            sqlite3_column_text(stmt, 2),
            sqlite3_column_text(stmt, 3),
            sqlite3_column_text(stmt, 4),
            sqlite3_column_text(stmt, 5),
            sqlite3_column_text(stmt, 6),
            sqlite3_column_text(stmt, 7)
        );
        strcat(json, row);
    }

    strcat(json, "]");
    sqlite3_finalize(stmt);
    return json;
}

int db_delete_student(sqlite3 *db, int id)
{
    const char *sql = "DELETE FROM students WHERE id=?";
    sqlite3_stmt *stmt;

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? 0 : -1;
}

int db_update_student(sqlite3 *db, int id,
    const char *name, const char *gender, const char *dob,
    const char *status, const char *guardian, const char *contact1, const char *contact2)
{
    const char *sql =
        "UPDATE students SET name=?, gender=?, date_of_birth=?, status=?, guardian_name=?, contact1=?, contact2=? WHERE id=?";
    sqlite3_stmt *stmt;

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, gender, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, dob, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, status, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, guardian, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, contact1, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, contact2 ? contact2 : "", -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 8, id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? 0 : -1;
}


char *db_search_students(sqlite3 *db, const char *field, const char *value)
{
    char sql[512];
    snprintf(sql, sizeof(sql),
             "SELECT id,name,gender,date_of_birth,status,guardian_name,contact1,contact2 "
             "FROM students WHERE %s LIKE ?", field);

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    char query[256];
    snprintf(query, sizeof(query), "%%%s%%", value); // partial match
    sqlite3_bind_text(stmt, 1, query, -1, SQLITE_STATIC);

    char *json = malloc(8192);
    strcpy(json, "[");
    int first = 1;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) strcat(json, ",");
        first = 0;

        char row[512];
        snprintf(row, sizeof(row),
            "{\"id\":%d,\"name\":\"%s\",\"gender\":\"%s\",\"date_of_birth\":\"%s\","
            "\"status\":\"%s\",\"guardian_name\":\"%s\",\"contact1\":\"%s\",\"contact2\":\"%s\"}",
            sqlite3_column_int(stmt, 0),
            sqlite3_column_text(stmt, 1),
            sqlite3_column_text(stmt, 2),
            sqlite3_column_text(stmt, 3),
            sqlite3_column_text(stmt, 4),
            sqlite3_column_text(stmt, 5),
            sqlite3_column_text(stmt, 6),
            sqlite3_column_text(stmt, 7)
        );
        strcat(json, row);
    }

    strcat(json, "]");
    sqlite3_finalize(stmt);
    return json;
}
