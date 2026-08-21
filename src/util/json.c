#include "json.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

int json_append_string(text_buffer *out, const char *value)
{
    const unsigned char *p = (const unsigned char *)fallback_text(value, "");

    if (text_buffer_append(out, "\"") < 0)
        return -1;

    for (; *p; p++) {
        switch (*p) {
        case '"':
            if (text_buffer_append(out, "\\\"") < 0)
                return -1;
            break;
        case '\\':
            if (text_buffer_append(out, "\\\\") < 0)
                return -1;
            break;
        case '\b':
            if (text_buffer_append(out, "\\b") < 0)
                return -1;
            break;
        case '\f':
            if (text_buffer_append(out, "\\f") < 0)
                return -1;
            break;
        case '\n':
            if (text_buffer_append(out, "\\n") < 0)
                return -1;
            break;
        case '\r':
            if (text_buffer_append(out, "\\r") < 0)
                return -1;
            break;
        case '\t':
            if (text_buffer_append(out, "\\t") < 0)
                return -1;
            break;
        default:
            if (*p < 0x20) {
                if (text_buffer_append(out, "\\u%04x", *p) < 0)
                    return -1;
            } else if (text_buffer_append(out, "%c", *p) < 0) {
                return -1;
            }
            break;
        }
    }

    return text_buffer_append(out, "\"");
}

static const char *skip_json_space(const char *p)
{
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;

    return p;
}

static bool json_value_ended(const char *p)
{
    p = skip_json_space(p);

    return *p == '\0' || *p == ',' || *p == '}' || *p == ']';
}

bool json_int_key_checked(const char *json, const char *key, int *out)
{
    const char *p = json_find_key(json, key);
    char *end;
    long value;

    if (!p)
        return false;

    p = json_after_colon(p);
    if (!p)
        return false;

    errno = 0;
    value = strtol(p, &end, 10);

    if (errno != 0 || end == p || value < INT_MIN || value > INT_MAX ||
        !json_value_ended(end))
        return false;

    *out = (int)value;
    return true;
}

int json_int_key(const char *json, const char *key, const int fallback)
{
    int value;

    if (!json_int_key_checked(json, key, &value))
        return fallback;

    return value;
}

bool json_bool_key_checked(const char *json, const char *key, bool *out)
{
    const char *p = json_find_key(json, key);

    if (!p)
        return false;

    p = json_after_colon(p);
    if (!p)
        return false;

    if (strncmp(p, "true", 4) == 0 && json_value_ended(p + 4)) {
        *out = true;
        return true;
    }

    if (strncmp(p, "false", 5) == 0 && json_value_ended(p + 5)) {
        *out = false;
        return true;
    }

    return false;
}

bool json_bool_key(const char *json, const char *key, const bool fallback)
{
    bool value;

    if (!json_bool_key_checked(json, key, &value))
        return fallback;

    return value;
}

bool json_string_key_checked(const char *json, const char *key, char *out, const size_t out_len)
{
    const char *p = json_find_key(json, key);

    if (!p || out_len == 0)
        return false;

    p = json_after_colon(p);
    if (!p || *p != '"')
        return false;

    p++;

    const char *end = strchr(p, '"');

    if (!end)
        return false;

    for (const char *q = p; q < end; q++) {
        if (*q == '\\' || (unsigned char)*q < 0x20)
            return false;
    }

    if (!json_value_ended(end + 1))
        return false;

    size_t len = (size_t)(end - p);

    if (len >= out_len)
        len = out_len - 1;

    memcpy(out, p, len);

    out[len] = '\0';
    return true;
}

void json_string_key(const char *json, const char *key, char *out, const size_t out_len)
{
    (void)json_string_key_checked(json, key, out, out_len);
}
