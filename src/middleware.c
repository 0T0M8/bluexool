#include "middleware.h"
#include <stdio.h>
#include <string.h>

#define MAX_MIDDLEWARE 20

static MiddlewareFunc middlewares[MAX_MIDDLEWARE];
static int middleware_count = 0;

// Register middleware
void middleware_register(MiddlewareFunc func) {
    if (middleware_count >= MAX_MIDDLEWARE) {
        printf("Max middleware reached!\n");
        return;
    }
    middlewares[middleware_count++] = func;
}

// Execute middleware chain
int middleware_execute(HttpRequest *req, HttpResponse *res) {
    for (int i = 0; i < middleware_count; i++) {
        if (!middlewares[i](req, res)) {
            return 0; // stop processing if middleware fails
        }
    }
    return 1; // continue to router
}
