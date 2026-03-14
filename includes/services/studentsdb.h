#ifndef STUDENTDB_H
#define STUDENTDB_H

#include <sqlite3.h>

/* Database initialization & closing */
int db_init(sqlite3 **db);
void db_close(sqlite3 *db);

/* CRUD operations */
int db_add_student(sqlite3 *db,
    const char *name, const char *gender, const char *dob,
    const char *status, const char *guardian, const char *contact1, const char *contact2);

char *db_get_students(sqlite3 *db);
int db_delete_student(sqlite3 *db, int id);

int db_update_student(sqlite3 *db, int id,
    const char *name, const char *gender, const char *dob,
    const char *status, const char *guardian, const char *contact1, const char *contact2);

/* Search students by name (partial match) */
char *db_search_students(sqlite3 *db, const char *field, const char *value);
#endif
