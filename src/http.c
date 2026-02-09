/* http.c - production ready */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include "http.h"

/* ================= HTTP ================= */

void send_response(int client, const char *type, const char *body)
{
    if (!body) body = "";

    char header[256];
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n\r\n",
        type, strlen(body));

    if (header_len < 0 || header_len >= sizeof(header)) {
        perror("Header snprintf error");
        close(client);
        return;
    }

    if (write(client, header, header_len) <= 0 ||
        write(client, body, strlen(body)) <= 0) {
        perror("Failed to write response");
        close(client);
        return;
    }
}

/* ================= JSON HELPERS ================= */

int json_get_string(const char *json, const char *key, char *out, int max)
{
    if (!json || !key || !out || max <= 0) return 0;

    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);

    char *p = strstr(json, pattern);
    if (!p) return 0;

    p = strchr(p + strlen(pattern), ':');
    if (!p) return 0;

    p = strchr(p, '"');
    if (!p) return 0;
    p++;

    char *end = strchr(p, '"');
    if (!end) return 0;

    int len = end - p;
    if (len >= max) len = max - 1;

    strncpy(out, p, len);
    out[len] = '\0';
    return 1;
}

int json_get_int(const char *json, const char *key, int *out)
{
    if (!json || !key || !out) return 0;

    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);

    char *p = strstr(json, pattern);
    if (!p) return 0;

    p = strchr(p + strlen(pattern), ':');
    if (!p) return 0;

    *out = atoi(p + 1);
    return 1;
}

/* ================= STATIC ================= */

const char *get_mime_type(const char *path)
{
    const char *ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream";

    if (!strcmp(ext, ".html")) return "text/html";
    if (!strcmp(ext, ".css"))  return "text/css";
    if (!strcmp(ext, ".js"))   return "application/javascript";
    if (!strcmp(ext, ".png"))  return "image/png";
    if (!strcmp(ext, ".jpg"))  return "image/jpeg";

    return "application/octet-stream";
}

int serve_static(int client, const char *url_path)
{
    if (!url_path) return 0;
    if (strstr(url_path, "..")) {
        send_response(client, "text/plain", "403 Forbidden");
        return 1;
    }

    char full[512];
    if (!strcmp(url_path, "/"))
        snprintf(full, sizeof(full), "static/index.html");
    else
        snprintf(full, sizeof(full), ".%s", url_path);

    struct stat st;
    if (stat(full, &st) < 0 || !S_ISREG(st.st_mode)) {
        perror("File stat error");
        return 0;
    }

    FILE *f = fopen(full, "rb");
    if (!f) {
        perror("Failed to open static file");
        return 0;
    }

    char *buf = malloc(st.st_size);
    if (!buf) {
        perror("Malloc failed for static file");
        fclose(f);
        return 0;
    }

    size_t n = fread(buf, 1, st.st_size, f);
    fclose(f);

    if (n != st.st_size) {
        perror("Failed to read full static file");
        free(buf);
        return 0;
    }

    char header[256];
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n\r\n",
        get_mime_type(full), st.st_size);

    if (write(client, header, header_len) <= 0 ||
        write(client, buf, st.st_size) <= 0) {
        perror("Failed to write static file to client");
        free(buf);
        close(client);
        return 1;
    }

    free(buf);
    return 1;
}

/* ================= HELPERS ================= */

char *get_body(const char *req)
{
    if (!req) return NULL;
    const char *p = strstr(req, "\r\n\r\n");
    return p ? strdup(p + 4) : NULL;
}

char *get_query_param(const char *req, const char *key)
{
    if (!req || !key) return NULL;

    const char *q = strchr(req, '?');
    if (!q) return NULL;

    char *query = strdup(q + 1);
    if (!query) return NULL;

    char *tok = strtok(query, "&");
    while (tok) {
        char k[64], v[256];
        if (sscanf(tok, "%63[^=]=%255s", k, v) == 2 && strcmp(k, key) == 0) {
            char *value = strdup(v);
            free(query);
            return value;
        }
        tok = strtok(NULL, "&");
    }

    free(query);
    return NULL;
}
