#ifndef MIDDLEWARE_H
#define MIDDLEWARE_H

#include "http.h"

typedef int (*Middleware)(HttpRequest *, HttpResponse *);

void middleware_register(Middleware m);
int middleware_execute(HttpRequest *req, HttpResponse *res);

/* add this line */
int auth_middleware(HttpRequest *req, HttpResponse *res);

#endif
