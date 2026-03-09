/* main.c - DirtBlue Students server entry point */
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include "router.h"
#include "studentsdb.h"

#define PORT 8081
#define BACKLOG 10
#define REQ_BUF_SIZE 4096

int main(void) {
    sqlite3 *db;
    if (db_init(&db) != 0) {
        fprintf(stderr, "Failed to initialize students database\n");
        return 1;
    }

    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        perror("socket");
        db_close(db);
        return 1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server);
        db_close(db);
        return 1;
    }

    if (listen(server, BACKLOG) < 0) {
        perror("listen");
        close(server);
        db_close(db);
        return 1;
    }

    printf("🚀 DirtBlue Students running at http://localhost:%d\n", PORT);

    while (1) {
        int client = accept(server, NULL, NULL);
        if (client < 0) {
            perror("accept");
            continue;
        }

        char req[REQ_BUF_SIZE] = {0};
        ssize_t n = read(client, req, sizeof(req) - 1);
        (void)n;  // suppress unused warning

        route(client, req, db);
        close(client);
    }

    db_close(db);
    close(server);
    return 0;
}
