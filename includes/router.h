#ifndef ROUTER_H
#define ROUTER_H

#include <sqlite3.h>

void route(int client, const char *req, sqlite3 *db);

#endif
