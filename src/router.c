/* router.c - production-ready */
#include <unistd.h> 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "router.h"
#include "http.h"
#include "studentsdb.h"

/* Utility: send JSON error with HTTP status */
static void send_json_error(int client, const char *message, int status) {
    char body[256];
    snprintf(body, sizeof(body), "{\"error\":\"%s\"}", message);
    char header[256];
    snprintf(header, sizeof(header),
             "HTTP/1.1 %d %s\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %ld\r\n\r\n",
             status, status == 404 ? "Not Found" : "Bad Request",
             strlen(body));
    write(client, header, strlen(header));
    write(client, body, strlen(body));
}

/* Main routing function */
void route(int client, const char *req, sqlite3 *db)
{
    if (!req || !db) {
        send_json_error(client, "Internal Server Error", 500);
        return;
    }

    char method[8], path[256];
    sscanf(req, "%7s %255s", method, path);

    /* ---------------- STATIC FILES ---------------- */
    if (!strcmp(method, "GET") && serve_static(client, path))
        return;

    /* ---------------- GET ALL STUDENTS ---------------- */
    if (!strcmp(method, "GET") && !strcmp(path, "/students")) {
        char *json = db_get_students(db);
        if (!json) send_json_error(client, "Failed to fetch students", 500);
        else {
            send_response(client, "application/json", json);
            free(json);
        }
        return;
    }

    /* ---------------- SEARCH STUDENTS ---------------- */
    if (!strcmp(method, "GET") && strstr(path, "/students/search")) {
        char *field = get_query_param(path, "field");
        char *value = get_query_param(path, "value");

        if (!field || !value) {
            send_json_error(client, "Missing 'field' or 'value'", 400);
        } else {
            char *json = db_search_students(db, field, value);
            if (!json) send_json_error(client, "Search failed", 500);
            else {
                send_response(client, "application/json", json);
                free(json);
            }
        }
        free(field);
        free(value);
        return;
    }

    /* ---------------- POST NEW STUDENT ---------------- */
    if (!strcmp(method, "POST") && !strcmp(path, "/students")) {
        char *body = get_body(req);
        if (!body) {
            send_json_error(client, "Missing body", 400);
            return;
        }

        char name[128], gender[16], dob[32], status[32], guardian[128], contact1[32], contact2[32];
        if (!json_get_string(body, "name", name, sizeof(name)) ||
            !json_get_string(body, "gender", gender, sizeof(gender)) ||
            !json_get_string(body, "date_of_birth", dob, sizeof(dob)) ||
            !json_get_string(body, "status", status, sizeof(status)) ||
            !json_get_string(body, "guardian_name", guardian, sizeof(guardian)) ||
            !json_get_string(body, "contact1", contact1, sizeof(contact1)) ||
            !json_get_string(body, "contact2", contact2, sizeof(contact2)) ||
            db_add_student(db, name, gender, dob, status, guardian, contact1, contact2) != 0)
        {
            send_json_error(client, "Failed to create student", 500);
        } else {
            send_response(client, "application/json", "{\"status\":\"created\"}");
        }

        free(body);
        return;
    }

    /* ---------------- PUT UPDATE STUDENT ---------------- */
    if (!strcmp(method, "PUT") && !strcmp(path, "/students")) {
        char *body = get_body(req);
        if (!body) {
            send_json_error(client, "Missing body", 400);
            return;
        }

        int id;
        char name[128], gender[16], dob[32], status[32], guardian[128], contact1[32], contact2[32];
        if (!json_get_int(body, "id", &id) ||
            !json_get_string(body, "name", name, sizeof(name)) ||
            !json_get_string(body, "gender", gender, sizeof(gender)) ||
            !json_get_string(body, "date_of_birth", dob, sizeof(dob)) ||
            !json_get_string(body, "status", status, sizeof(status)) ||
            !json_get_string(body, "guardian_name", guardian, sizeof(guardian)) ||
            !json_get_string(body, "contact1", contact1, sizeof(contact1)) ||
            !json_get_string(body, "contact2", contact2, sizeof(contact2)) ||
            db_update_student(db, id, name, gender, dob, status, guardian, contact1, contact2) != 0)
        {
            send_json_error(client, "Failed to update student", 500);
        } else {
            send_response(client, "application/json", "{\"status\":\"updated\"}");
        }

        free(body);
        return;
    }

    /* ---------------- DELETE STUDENT ---------------- */
    if (!strcmp(method, "DELETE") && strstr(path, "/students")) {
        char *id_str = get_query_param(path, "id");
        if (!id_str) {
            send_json_error(client, "Missing 'id'", 400);
            return;
        }

        int id = atoi(id_str);
        free(id_str);

        if (id <= 0 || db_delete_student(db, id) != 0) {
            send_json_error(client, "Failed to delete student", 500);
        } else {
            send_response(client, "application/json", "{\"status\":\"deleted\"}");
        }
        return;
    }

    /* ---------------- DEFAULT 404 ---------------- */
    send_json_error(client, "Not Found", 404);
}
