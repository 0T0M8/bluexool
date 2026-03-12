#ifndef ROUTER_H
#define ROUTER_H

#include "http.h"

typedef void (*RouteHandler)(HttpRequest *req, HttpResponse *res);

void router_register(const char *path, RouteHandler handler);
void router_handle(HttpRequest *req, HttpResponse *res);

#endif
