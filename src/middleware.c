#include "middleware.h"
#include "services.h"
#include <string.h>
#include <stdio.h>

#define MAX_MIDDLEWARE 16

static Middleware middlewares[MAX_MIDDLEWARE];
static int middleware_count = 0;

void middleware_register(Middleware m)
{
    if (middleware_count < MAX_MIDDLEWARE) {
        middlewares[middleware_count++] = m;
    }
}

int middleware_execute(HttpRequest *req, HttpResponse *res)
{
    for (int i = 0; i < middleware_count; i++) {
        if (!middlewares[i](req, res)) {
            return 0;
        }
    }
    return 1;
}

int auth_middleware(HttpRequest *req, HttpResponse *res) {

    if (strcmp(req->path, "/dashboard") == 0) {

        if (strstr(req->headers, "username:") == NULL) {
            res->status = 401;
            strcpy(res->body, "Unauthorized");
            return 0;
        }
    }

    return 1;
}
