/* http.c */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "http.h"

/* ================= HTTP ================= */

void send_response(int client, const char *type, const char *body)
{
    char header[256];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n\r\n",
        type, strlen(body));

    write(client, header, strlen(header));
    write(client, body, strlen(body));
}

/* ================= JSON HELPERS ================= */

int json_get_string(const char *json, const char *key, char *out, int max)
{
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
    if (strstr(url_path, "..")) {
        send_response(client, "text/plain", "403 Forbidden");
        return 1;
    }

    char full[512];
    if (!strcmp(url_path, "/"))
        strcpy(full, "static/index.html");
    else
        snprintf(full, sizeof(full), ".%s", url_path);

    struct stat st;
    if (stat(full, &st) < 0 || !S_ISREG(st.st_mode))
        return 0;

    FILE *f = fopen(full, "rb");
    if (!f) return 0;

    char *buf = malloc(st.st_size);
    ssize_t n = fread(buf, 1, st.st_size, f);
    if (n != st.st_size) {
         // handle error
    }
    fclose(f);

    char header[256];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n\r\n",
        get_mime_type(full), st.st_size);

    n = write(client, header, strlen(header));


    n = write(client, buf, st.st_size);
    
    free(buf);
    return 1;
}

/* ================= HELPERS ================= */

char *get_body(const char *req)
{
    const char *p = strstr(req, "\r\n\r\n");
    return p ? strdup(p + 4) : NULL;
}

char *get_query_param(const char *req, const char *key)
{
    const char *q = strchr(req, '?');
    if (!q) return NULL;

    char *query = strdup(q + 1);
    char *tok = strtok(query, "&");

    while (tok) {
        char k[64], v[128];
        if (sscanf(tok, "%63[^=]=%127s", k, v) == 2 &&
            strcmp(k, key) == 0) {
            free(query);
            return strdup(v);
        }
        tok = strtok(NULL, "&");
    }

    free(query);
    return NULL;
}
