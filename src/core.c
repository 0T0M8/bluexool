#include "core.h"
#include "services.h"
#include "middleware.h"
#include "router.h"
#include <stdio.h>
#include <string.h>

// Middleware
static int logger(HttpRequest *req, HttpResponse *res) {
    printf("[Middleware] %s %s\n", req->method, req->path);
    return 1;
}

// Route handlers
static void root_handler(HttpRequest *req, HttpResponse *res)
{
    res->status = 200;
    strcpy(res->body, "Bluexool server is running!");
}
/*
static void register_handler(HttpRequest *req, HttpResponse *res) {
    char username[64], password[64];
    sscanf(req->body, "username=%s&password=%s", username, password);

    if (services_create_user(username, password)) {
        strcpy(res->body, "User registered successfully");
        res->status = 200;
    } else {
        strcpy(res->body, "Registration failed");
        res->status = 400;
    }
}
*/

/*
static void register_handler(HttpRequest *req, HttpResponse *res)
{
    char username[64];
    char password[64];

    sscanf(req->body, "username=%63[^&]&password=%63s", username, password);

    printf("Parsed username: %s\n", username);
    printf("Parsed password: %s\n", password);

    if (services_create_user(username, password)) {
        res->status = 200;
        strcpy(res->body, "User registered successfully");
    } else {
        res->status = 400;
        strcpy(res->body, "Registration failed");
    }
}


static void login_handler(HttpRequest *req, HttpResponse *res) {
    //char username[64], password[64];
    //sscanf(req->body, "username=%s&password=%s", username, password);

    char username[64], password[64];

    sscanf(req->body, "username=%63[^&]&password=%63s", username, password);

    printf("Parsed username: %s\n", username);
    printf("Parsed password: %s\n", password);

    if (services_validate_user(username, password)) {
        strcpy(res->body, "Login success!");
        res->status = 200;
    } else {
        strcpy(res->body, "Invalid credentials");
        res->status = 401;
    }
}
*/
static void dashboard_handler(HttpRequest *req, HttpResponse *res) {
    char username[64];
    sscanf(req->headers, "username=%s", username);
    User u;
    if (services_get_user(username, &u)) {
        sprintf(res->body, "Welcome to your dashboard, %s!", u.username);
        res->status = 200;
    } else {
        strcpy(res->body, "User not found");
        res->status = 404;
    }
}
/*
void core_run(int port) {
    services_init();

    middleware_register(logger);
    middleware_register(auth_middleware);
    
    router_register("/", root_handler);
    router_register("/register", register_handler);
    router_register("/login", login_handler);
    router_register("/dashboard", dashboard_handler);

    printf("Server running on port %d...\n", port);
    http_start(port);
}*/

// Register Handler
// -----------------------------
static void register_handler(HttpRequest *req, HttpResponse *res)
{
    char username[64];
    char password[64];

    // Parse POST body: username=...&password=...
    sscanf(req->body, "username=%63[^&]&password=%63s", username, password);

    printf("[Register] Parsed username: %s\n", username);
    printf("[Register] Parsed password: %s\n", password);

    // Create user in database
    if (services_create_user(username, password)) {
        res->status = 200;
        strcpy(res->body, "User registered successfully");
    } else {
        res->status = 400;
        strcpy(res->body, "Registration failed");
    }
}

// -----------------------------
// Login Handler
// -----------------------------
static void login_handler(HttpRequest *req, HttpResponse *res)
{
    char username[64];
    char password[64];

    // Parse POST body the same way as registration
    sscanf(req->body, "username=%63[^&]&password=%63s", username, password);

    printf("[Login] Parsed username: %s\n", username);
    printf("[Login] Parsed password: %s\n", password);

    // Validate user with bcrypt against password_hash
    if (services_validate_user(username, password)) {
        res->status = 200;
        strcpy(res->body, "Login success!");
    } else {
        res->status = 401;
        strcpy(res->body, "Invalid credentials");
    }
}


void core_run(int port) {
    services_init();
                                                    middleware_register(logger);
    middleware_register(auth_middleware);

    router_register("/", root_handler);
    router_register("/register", register_handler);
    router_register("/login", login_handler);
    router_register("/dashboard", dashboard_handler);

    printf("Server running on port %d...\n", port);
    http_start(port);
}
