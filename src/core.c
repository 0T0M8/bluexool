#include "core.h"
#include "services.h"
#include "middleware.h"
#include "router.h"
#include <stdio.h>
#include <string.h>

// Example middleware: log all requests
static int logger(HttpRequest *req, HttpResponse *res) {
    printf("[Core Middleware] %s %s\n", req->method, req->path);
    return 1; // continue to router
}

// Example route handler
static void hello_handler(HttpRequest *req, HttpResponse *res) {
    strcpy(res->body, "Hello from Core!");
    res->status = 200;
}

void core_run(int port) {
    printf("Initializing Core modules...\n");

    // 1️⃣ Initialize Services (DB, etc.)
    services_init();

    // 2️⃣ Register middleware
    middleware_register(logger);

    // 3️⃣ Register routes
    router_register("/", hello_handler);

    // 4️⃣ Start HTTP server
    printf("Starting server on port %d...\n", port);
    http_start(port);
}
