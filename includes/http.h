#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

typedef struct {
    char method[8];
    char path[256];
    char body[1024];
} HttpRequest;

typedef struct {
    int status;
    char body[1024];
} HttpResponse;

void http_start(int port);
HttpRequest http_parse_request(const char *raw_request);
void http_send_response(int client_socket, HttpResponse *res);

#endif
