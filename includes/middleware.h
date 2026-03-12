#ifndef MIDDLEWARE_H
#define MIDDLEWARE_H

#include "http.h"
#include "router.h"

// Middleware function type: returns 1 to continue, 0 to stop
typedef int (*MiddlewareFunc)(HttpRequest *req, HttpResponse *res);

// Register a middleware function
void middleware_register(MiddlewareFunc func);

// Execute all middleware for a request
int middleware_execute(HttpRequest *req, HttpResponse *res);

#endif
