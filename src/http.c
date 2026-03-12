#include "http.h"
#include "router.h"
#include "middleware.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

int server_fd = -1; // global server socket

void handle_sigint(int sig) {
    if (server_fd != -1) {
        close(server_fd);
        printf("\nServer socket closed. Exiting.\n");
    }
    exit(0);
}

void http_parse_request(char *buffer, HttpRequest *req) {
    sscanf(buffer, "%7s %127s", req->method, req->path);

    char *header_start = strstr(buffer, "\r\n") + 2;
    char *body_start = strstr(buffer, "\r\n\r\n");

    if (body_start) {
        strncpy(req->headers, header_start, body_start - header_start);
        req->headers[body_start - header_start] = '\0';

        strcpy(req->body, body_start + 4);
    } else {
        strcpy(req->headers, header_start);
        req->body[0] = '\0';
    }
}

/*
HttpRequest http_parse_request(const char *raw_request) {
    HttpRequest req = {0};
    // parse method and path
    sscanf(raw_request, "%s %s", req.method, req.path);

    // parse body if exists
    const char *body_ptr = strstr(raw_request, "\r\n\r\n");
    if (body_ptr) {
        strncpy(req.body, body_ptr + 4, sizeof(req.body) - 1);
    }

    return req;
}
*/
void http_start(int port) {
    struct sockaddr_in addr;
    char buffer[2048];

    // Register Ctrl+C handler
    signal(SIGINT, handle_sigint);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket failed"); exit(EXIT_FAILURE); }

    // Allow immediate reuse of the port
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed"); exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind failed"); exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) { perror("listen failed"); exit(EXIT_FAILURE); }

    printf("HTTP server running on port %d...\n", port);

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) continue;

        int n = read(client_fd, buffer, sizeof(buffer) - 1);
        if (n <= 0) { close(client_fd); continue; }
        buffer[n] = '\0';

        // After
HttpRequest req;
http_parse_request(buffer, &req);
HttpResponse res = {0};
//router_handle(&req, &res);
if (middleware_execute(&req, &res)) {
    router_handle(&req, &res);
}
// else: middleware already set the response (e.g., 401 Unauthorized)
char response[2048];
sprintf(response,
        "HTTP/1.1 %d OK\r\nContent-Length: %lu\r\nContent-Type: text/plain\r\n\r\n%s",
        res.status,
        strlen(res.body),
        res.body);

write(client_fd, response, strlen(response));
        close(client_fd);
    }
}

