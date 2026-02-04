#ifndef HTTP_H
#define HTTP_H

void send_response(int client, const char *type, const char *body);
char *get_body(const char *req);
char *get_query_param(const char *req, const char *key);

/* NEW: simple JSON helpers */
int json_get_string(const char *json, const char *key, char *out, int max);
int json_get_int(const char *json, const char *key, int *out);

/* static */
const char *get_mime_type(const char *path);
int serve_static(int client, const char *url_path);

#endif
