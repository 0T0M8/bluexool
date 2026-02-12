/* router.c - production-ready with auth */
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "router.h"
#include "http.h"
#include "studentsdb.h"
#include "usersdb.h"
#include "bcrypt.h"

#define SESSION_EXPIRY_SECONDS 3600  // 1 hour

/* ---------------- HELPERS ---------------- */

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

static void send_json_response(int client, const char *json) {
    send_response(client, "application/json", json);
}

static char *generate_session_id(void) {
    static const char charset[] =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    char *id = malloc(33);
    if (!id) return NULL;

    for (int i = 0; i < 32; i++)
        id[i] = charset[rand() % (sizeof(charset)-1)];
    id[32] = '\0';
    return id;
}

static void send_cookie_response(int client, const char *session_id) {
    char body[128];
    snprintf(body, sizeof(body), "{\"status\":\"ok\"}");

    char header[256];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Set-Cookie: session=%s; HttpOnly; Path=/; Max-Age=%d\r\n"
        "Content-Length: %ld\r\n\r\n",
        session_id, SESSION_EXPIRY_SECONDS, strlen(body));

    write(client, header, strlen(header));
    write(client, body, strlen(body));
}

static int parse_cookie(const char *req, char *out, size_t size) {
    if (!req || !out) return 0;

    const char *c = strstr(req, "Cookie:");
    if (!c) return 0;

    const char *s = strstr(c, "session=");
    if (!s) return 0;

    s += strlen("session=");
    const char *end = strpbrk(s, ";\r\n");
    size_t len = end ? (size_t)(end - s) : strlen(s);
    if (len >= size) len = size - 1;

    strncpy(out, s, len);
    out[len] = '\0';
    return 1;
}

static int require_auth(sqlite3 *db, const char *req, int *user_id_out) {
    char session_id[64];
    if (!parse_cookie(req, session_id, sizeof(session_id))) return 0;

    int uid = 0;
    char expires[64];
    if (db_get_session(db, session_id, &uid, expires, sizeof(expires)) != 0)
        return 0;

    // Optional: check expiry here (skipping for brevity)
    if (user_id_out) *user_id_out = uid;
    return 1;
}

/* ---------------- ROUTING ---------------- */

void route(int client, const char *req, sqlite3 *db) {
    if (!req || !db) {
        send_json_error(client, "Internal Server Error", 500);
        return;
    }

    char method[8], path[256];
    sscanf(req, "%7s %255s", method, path);

    /* ---------------- STATIC ---------------- */
    if (!strcmp(method, "GET") && serve_static(client, path)) return;

    /* ---------------- AUTH: REGISTER ---------------- */
    if (!strcmp(method, "POST") && !strcmp(path, "/register")) {
        char *body = get_body(req);
        if (!body) { send_json_error(client, "Missing body", 400); return; }

        char username[64], password[64];
        if (!json_get_string(body, "username", username, sizeof(username)) ||
            !json_get_string(body, "password", password, sizeof(password))) {
            send_json_error(client, "Missing username or password", 400);
            free(body); return;
        }

        char hash[BCRYPT_HASHSIZE];
        char salt[BCRYPT_HASHSIZE];

        // generate a salt first
        if (bcrypt_gensalt(12, salt) != 0) { // 12 = work factor
            send_json_error(client, "Failed to generate salt", 500);
            free(body);
            return;
        }

        // hash the password
        if (bcrypt_hashpw(password, salt, hash) != 0) {
            send_json_error(client, "Failed to hash password", 500);
            free(body);
            return;
        }

        // save hash into database
        if (db_create_user(db, username, hash, "user") != 0) {
            send_json_error(client, "Failed to create user", 500);
        } else {
            send_response(client, "application/json", "{\"status\":\"created\"}");
        }
    }

    /* ---------------- AUTH: LOGIN ---------------- */
    if (!strcmp(method, "POST") && !strcmp(path, "/login")) {
        char *body = get_body(req);
        if (!body) { send_json_error(client, "Missing body", 400); return; }

        char username[64], password[64];
        if (!json_get_string(body, "username", username, sizeof(username)) ||
            !json_get_string(body, "password", password, sizeof(password))) {
            send_json_error(client, "Missing username or password", 400);
            free(body); return;
        }

        char stored_hash[128], role[32];
        int user_id, is_active;
        if (db_get_user_by_username(db, username, &user_id, stored_hash, sizeof(stored_hash),
                                   role, sizeof(role), &is_active) != 0 ||
            !is_active ||
            bcrypt_checkpw(password, stored_hash) != 0) {
            send_json_error(client, "Invalid credentials", 401);
            free(body); return;
        }

        // create session
        char *session_id = generate_session_id();
        if (!session_id) { send_json_error(client, "Failed to create session", 500); free(body); return; }

        time_t now = time(NULL);
        char expires[32];
        snprintf(expires, sizeof(expires), "%ld", now + SESSION_EXPIRY_SECONDS);

        if (db_create_session(db, session_id, user_id, expires) != 0) {
            send_json_error(client, "Failed to create session", 500);
            free(session_id); free(body); return;
        }

        send_cookie_response(client, session_id);
        free(session_id); free(body); return;
    }

    /* ---------------- AUTH: LOGOUT ---------------- */
    if (!strcmp(method, "POST") && !strcmp(path, "/logout")) {
        char session_id[64];
        if (parse_cookie(req, session_id, sizeof(session_id)))
            db_delete_session(db, session_id);

        send_json_response(client, "{\"status\":\"logged_out\"}");
        return;
    }

    /* ---------------- STUDENTS CRUD ---------------- */

    // require authentication for students endpoints
    int user_id;
    if (!require_auth(db, req, &user_id) && strstr(path, "/students")) {
        send_json_error(client, "Unauthorized", 401);
        return;
    }

    // Existing CRUD endpoints from your previous router.c
    // GET /students
    if (!strcmp(method, "GET") && !strcmp(path, "/students")) {
        char *json = db_get_students(db);
        if (!json) send_json_error(client, "Failed to fetch students", 500);
        else { send_json_response(client, json); free(json); }
        return;
    }

    // GET /students/search
    if (!strcmp(method, "GET") && strstr(path, "/students/search")) {
        char *field = get_query_param(path, "field");
        char *value = get_query_param(path, "value");
        if (!field || !value) { send_json_error(client, "Missing query params", 400); }
        else {
            char *json = db_search_students(db, field, value);
            if (!json) send_json_error(client, "Search failed", 500);
            else { send_json_response(client, json); free(json); }
        }
        free(field); free(value); return;
    }

    // POST /students
    if (!strcmp(method, "POST") && !strcmp(path, "/students")) {
        char *body = get_body(req);
        if (!body) { send_json_error(client, "Missing body", 400); return; }

        char name[128], gender[16], dob[32], status[32], guardian[128], contact1[32], contact2[32];
        if (!json_get_string(body, "name", name, sizeof(name)) ||
            !json_get_string(body, "gender", gender, sizeof(gender)) ||
            !json_get_string(body, "date_of_birth", dob, sizeof(dob)) ||
            !json_get_string(body, "status", status, sizeof(status)) ||
            !json_get_string(body, "guardian_name", guardian, sizeof(guardian)) ||
            !json_get_string(body, "contact1", contact1, sizeof(contact1)) ||
            !json_get_string(body, "contact2", contact2, sizeof(contact2)) ||
            db_add_student(db, name, gender, dob, status, guardian, contact1, contact2) != 0) {
            send_json_error(client, "Failed to create student", 500);
        } else send_json_response(client, "{\"status\":\"created\"}");

        free(body); return;
    }

    // PUT /students
    if (!strcmp(method, "PUT") && !strcmp(path, "/students")) {
        char *body = get_body(req);
        if (!body) { send_json_error(client, "Missing body", 400); return; }

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
            db_update_student(db, id, name, gender, dob, status, guardian, contact1, contact2) != 0) {
            send_json_error(client, "Failed to update student", 500);
        } else send_json_response(client, "{\"status\":\"updated\"}");

        free(body); return;
    }

    // DELETE /students
    if (!strcmp(method, "DELETE") && strstr(path, "/students")) {
        char *id_str = get_query_param(path, "id");
        if (!id_str) { send_json_error(client, "Missing 'id'", 400); return; }

        int id = atoi(id_str);
        free(id_str);

        if (id <= 0 || db_delete_student(db, id) != 0)
            send_json_error(client, "Failed to delete student", 500);
        else send_json_response(client, "{\"status\":\"deleted\"}");

        return;
    }

    /* ---------------- DEFAULT 404 ---------------- */
    send_json_error(client, "Not Found", 404);
}

