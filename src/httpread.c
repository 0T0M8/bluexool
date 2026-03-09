/* httpread.c */
/* http_read.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define INITIAL_BUF 4096

/* Extract Content-Length header */
static int get_content_length(const char *req)
{
    const char *p = strstr(req, "Content-Length:");
    if (!p) return 0;

    p += strlen("Content-Length:");
    while (*p == ' ') p++;

    return atoi(p);
}

/* Read full HTTP request */
char *read_http_request(int client)
{
    int buf_size = INITIAL_BUF;
    int total = 0;

    char *buf = malloc(buf_size + 1);
    if (!buf) return NULL;

    while (1)
    {
        ssize_t n = read(client, buf + total, buf_size - total);
        if (n <= 0)
            break;

        total += n;

        buf[total] = '\0';

        /* Check if headers finished */
        char *headers_end = strstr(buf, "\r\n\r\n");

        if (headers_end)
        {
            int header_len = headers_end + 4 - buf;
            int content_len = get_content_length(buf);

            int expected = header_len + content_len;

            if (total >= expected)
                break;
        }

        /* Expand buffer if needed */
        if (total >= buf_size - 1)
        {
            buf_size *= 2;
            char *tmp = realloc(buf, buf_size + 1);
            if (!tmp)
            {
                free(buf);
                return NULL;
            }
            buf = tmp;
        }
    }

    buf[total] = '\0';
    return buf;
}
