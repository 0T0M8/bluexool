#include "router.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_ROUTES 50

typedef struct {
    char path[256];
    RouteHandler handler;
} Route;

static Route routes[MAX_ROUTES];
static int route_count = 0;

void router_register(const char *path, RouteHandler handler) {
    if (route_count >= MAX_ROUTES) {
        printf("Max routes reached!\n");
        return;
    }
    strncpy(routes[route_count].path, path, sizeof(routes[route_count].path) - 1);
    routes[route_count].handler = handler;
    route_count++;
}

void router_handle(HttpRequest *req, HttpResponse *res) {
    for (int i = 0; i < route_count; i++) {
        if (strcmp(req->path, routes[i].path) == 0) {
            routes[i].handler(req, res);
            return;
        }
    }
    // Default response if no route matches
    res->status = 404;
    snprintf(res->body, sizeof(res->body), "Route %s not found", req->path);
}
