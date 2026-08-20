#include "json.h"

#include <stdio.h>
#include <string.h>

const char *json_find_key(const char *json, const char *key)
{
    char needle[96];

    snprintf(needle, sizeof(needle), "\"%s\"", key);

    return strstr(json, needle);
}

const char *json_after_colon(const char *p)
{
    p = strchr(p, ':');

    if (!p)
        return NULL;

    p++;

    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;

    return p;
}

const char *json_find_array(const char *json, const char *key, const char **end_out)
{
    const char *p = json_find_key(json, key);
    int depth = 0;

    if (!p || !end_out)
        return NULL;

    p = json_after_colon(p);
    if (!p || *p != '[')
        return NULL;

    for (const char *q = p; *q; q++) {
        if (*q == '[') {
            depth++;
        } else if (*q == ']') {
            depth--;
            if (depth == 0) {
                *end_out = q;
                return p + 1;
            }
        }
    }

    return NULL;
}

const char *json_next_object(const char *p, const char *end, const char **obj_end)
{
    int depth = 0;
    const char *start = NULL;

    if (!p || !end || !obj_end)
        return NULL;

    for (; p < end && *p; p++) {
        if (*p == '{') {
            if (depth == 0)
                start = p;
            depth++;
        } else if (*p == '}') {
            depth--;
            if (depth == 0 && start) {
                *obj_end = p + 1;
                return start;
            }
        }
    }

    return NULL;
}

void json_copy_slice(const char *start, const char *end, char *out,
                     const size_t out_len)
{
    size_t len;

    if (!out || out_len == 0)
        return;

    if (!start || !end || end < start) {
        out[0] = '\0';
        return;
    }

    len = (size_t)(end - start);
    if (len >= out_len)
        len = out_len - 1;

    memcpy(out, start, len);
    out[len] = '\0';
}

const char *json_object_with_id(const char *json, const char *id)
{
    char needle[64];

    snprintf(needle, sizeof(needle), "\"id\": \"%s\"", id);

    return strstr(json, needle);
}
