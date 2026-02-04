#ifndef DB_STUDENTS_H
#define DB_STUDENTS_H

#include <sqlite3.h>

// Initialize and close the students database
int db_init(sqlite3 **db);
void db_close(sqlite3 *db);

// -------------------- CRUD --------------------

// Add a new student
int db_add_student(
    sqlite3 *db,
    const char *name,
    const char *gender,
    const char *date_of_birth,
    const char *status,
    const char *guardian_name,
    const char *contact1,
    const char *contact2
);

// Get all students
// Returns a malloc'd JSON string. Caller must free().
char *db_get_students(sqlite3 *db);

// Delete a student by ID
int db_delete_student(sqlite3 *db, int id);

// Update a student by ID
int db_update_student(
    sqlite3 *db,
    int id,
    const char *name,
    const char *gender,
    const char *date_of_birth,
    const char *status,
    const char *guardian_name,
    const char *contact1,
    const char *contact2
);

// -------------------- SEARCH --------------------

// Search students by any column (e.g., name, guardian_name, status)
// Returns a malloc'd JSON string. Caller must free().
char *db_search_students(sqlite3 *db, const char *field, const char *value);

#endif // DB_STUDENTS_H
