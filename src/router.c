#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "router.h"
#include "http.h"
#include "studentsdb.h"

void route(int client, const char *req, sqlite3 *db)
{
    char method[8], path[256];
    sscanf(req, "%7s %255s", method, path);

    /* STATIC FILES */
    if (!strcmp(method, "GET") && serve_static(client, path))
        return;

    /* GET all students */
    if (!strcmp(method, "GET") && !strcmp(path, "/students")) {
        char *json = db_get_students(db);
        send_response(client, "application/json", json);
        free(json);
        return;
    }

    /* GET search */
    if (!strcmp(method, "GET") && strstr(path, "/students/search")) {
        char *field = get_query_param(path, "field");
        char *value = get_query_param(path, "value");

        if (field && value) {
            char *json = db_search_students(db, field, value);
            send_response(client, "application/json", json);
            free(json);
        } else {
            send_response(client, "application/json", "{\"error\":\"missing field or value\"}");
        }

        free(field);
        free(value);
        return;
    }

    /* POST new student (JSON) */
    if (!strcmp(method, "POST") && !strcmp(path, "/students")) {
        char *body = get_body(req);
        char name[128], gender[16], dob[32], status[32], guardian[128], contact1[32], contact2[32];

        if (body &&
            json_get_string(body, "name", name, sizeof(name)) &&
            json_get_string(body, "gender", gender, sizeof(gender)) &&
            json_get_string(body, "date_of_birth", dob, sizeof(dob)) &&
            json_get_string(body, "status", status, sizeof(status)) &&
            json_get_string(body, "guardian_name", guardian, sizeof(guardian)) &&
            json_get_string(body, "contact1", contact1, sizeof(contact1)) &&
            json_get_string(body, "contact2", contact2, sizeof(contact2)) &&
            db_add_student(db, name, gender, dob, status, guardian, contact1, contact2) == 0)
        {
            send_response(client, "application/json", "{\"status\":\"created\"}");
        } else {
            send_response(client, "application/json", "{\"error\":\"bad json\"}");
        }

        free(body);
        return;
    }

    /* PUT update student (JSON) */
    if (!strcmp(method, "PUT") && !strcmp(path, "/students")) {
        char *body = get_body(req);
        int id;
        char name[128], gender[16], dob[32], status[32], guardian[128], contact1[32], contact2[32];

        if (body &&
            json_get_int(body, "id", &id) &&
            json_get_string(body, "name", name, sizeof(name)) &&
            json_get_string(body, "gender", gender, sizeof(gender)) &&
            json_get_string(body, "date_of_birth", dob, sizeof(dob)) &&
            json_get_string(body, "status", status, sizeof(status)) &&
            json_get_string(body, "guardian_name", guardian, sizeof(guardian)) &&
            json_get_string(body, "contact1", contact1, sizeof(contact1)) &&
            json_get_string(body, "contact2", contact2, sizeof(contact2)) &&
            db_update_student(db, id, name, gender, dob, status, guardian, contact1, contact2) == 0)
        {
            send_response(client, "application/json", "{\"status\":\"updated\"}");
        } else {
            send_response(client, "application/json", "{\"error\":\"bad json\"}");
        }

        free(body);
        return;
    }

    /* DELETE student */
    if (!strcmp(method, "DELETE") && strstr(path, "/students")) {
        char *id_str = get_query_param(path, "id");
        int id = id_str ? atoi(id_str) : -1;
        free(id_str);

        if (id > 0 && db_delete_student(db, id) == 0)
            send_response(client, "application/json", "{\"status\":\"deleted\"}");
        else
            send_response(client, "application/json", "{\"error\":\"delete failed\"}");
        return;
    }

    /* Default 404 */
    send_response(client, "text/plain", "404 Not Found");
}
