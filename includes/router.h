#ifndef ROUTER_H
#define ROUTER_H

#include <sqlite3.h>

/*
 * ================= ROUTER =================
 *
 * Handles all incoming HTTP requests for DirtBlue Students.
 * Dispatches requests to the correct handlers: static files, students CRUD, and (future) auth endpoints.
 *
 * Parameters:
 *   client - socket file descriptor of the connected client
 *   req    - raw HTTP request string
 *   db     - pointer to the initialized SQLite3 database
 *
 * ---------------- ROUTES SUPPORTED ----------------
 *
 * 1. Static Files
 *    GET /                  -> serves index.html
 *    GET /<file>            -> serves static files from ./static
 *
 * 2. Students CRUD
 *    GET  /students                     -> returns JSON array of all students
 *    GET  /students/search?field=...&value=... -> returns JSON array matching query
 *    POST /students
 *        Body (JSON):
 *        {
 *          "name": "John Doe",
 *          "gender": "Male",
 *          "date_of_birth": "2005-06-01",
 *          "status": "Active",
 *          "guardian_name": "Jane Doe",
 *          "contact1": "123456789",
 *          "contact2": "987654321"
 *        }
 *        -> adds student, returns {"status":"created"}
 *
 *    PUT  /students
 *        Body (JSON):
 *        {
 *          "id": 1,
 *          "name": "John Doe",
 *          "gender": "Male",
 *          "date_of_birth": "2005-06-01",
 *          "status": "Active",
 *          "guardian_name": "Jane Doe",
 *          "contact1": "123456789",
 *          "contact2": "987654321"
 *        }
 *        -> updates student, returns {"status":"updated"}
 *
 *    DELETE /students?id=<id> -> deletes student, returns {"status":"deleted"}
 *
 * 3. Auth Endpoints (planned / to be implemented)
 *    POST /register  -> register new user
 *    POST /login     -> login and create session
 *    POST /logout    -> invalidate session
 *
 * ---------------- ERRORS ----------------
 *    Returns JSON on errors:
 *    {
 *       "error": "<error message>"
 *    }
 *    HTTP status codes used:
 *      400 - Bad Request (invalid/missing input)
 *      404 - Not Found (route not found)
 *      500 - Internal Server Error (database or unexpected failure)
 */
void route(int client, const char *req, sqlite3 *db);

#endif /* ROUTER_H */
