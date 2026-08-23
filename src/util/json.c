#include "json.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

/**
 * Append a JSON-escaped string.
 *
 * Status replies are assembled manually, so this covers the JSON escapes the
 * daemon can emit and converts remaining control bytes to unicode escapes.
 */
int json_append_string(text_buffer* out, const char* value)
{
    const unsigned char* p = (const unsigned char*)fallback_text(value, "");

    if (text_buffer_append(out, "\"") < 0)
        return -1;

    for (; *p; p++)
    {
        switch (*p)
        {
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
            default: {
                const int append_result = *p < 0x20 ? text_buffer_append(out, "\\u%04x", *p) : text_buffer_append(out, "%c", *p);

                if (append_result < 0)
                    return -1;

                break;
            }
        }
    }

    return text_buffer_append(out, "\"");
}

/**
 * Skip JSON whitespace.
 *
 * The model parser only accepts JSON's four whitespace bytes; keeping this
 * narrow avoids locale-dependent parsing behavior.
 */
static const char* skip_json_space(const char* p)
{
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;

    return p;
}

/**
 * Return whether a primitive JSON value is followed by a valid delimiter.
 *
 * This prevents numeric and boolean parsers from accepting prefixes such as
 * `12abc` or `trueish`.
 */
static bool json_value_ended(const char* p)
{
    p = skip_json_space(p);

    return *p == '\0' || *p == ',' || *p == '}' || *p == ']';
}

/**
 * Read and validate an integer JSON key.
 *
 * The parser rejects missing keys, overflow, trailing garbage, and values that
 * cannot fit in the daemon's integer fields.
 */
bool json_int_key_checked(const char* json, const char* key, int* out)
{
    char* end;
    const char* p = json_find_key(json, key);

    if (!p)
        return false;

    p = json_after_colon(p);

    if (!p)
        return false;

    errno = 0;
    const long value = strtol(p, &end, 10);

    if (errno != 0 || end == p || value < INT_MIN || value > INT_MAX || !json_value_ended(end))
        return false;

    *out = (int)value;

    return true;
}

/**
 * Read an integer JSON key.
 *
 * Optional integer fields use this wrapper to keep fallback handling at the
 * field declaration site.
 */
int json_int_key(const char* json, const char* key, const int fallback)
{
    int value;

    if (!json_int_key_checked(json, key, &value))
        return fallback;

    return value;
}

/**
 * Read and validate a boolean JSON key.
 *
 * Only exact JSON booleans are accepted, so partial words and quoted booleans
 * are rejected.
 */
bool json_bool_key_checked(const char* json, const char* key, bool* out)
{
    const char* p = json_find_key(json, key);

    if (!p)
        return false;

    p = json_after_colon(p);

    if (!p)
        return false;

    if (strncmp(p, "true", 4) == 0 && json_value_ended(p + 4))
    {
        *out = true;

        return true;
    }

    if (strncmp(p, "false", 5) == 0 && json_value_ended(p + 5))
    {
        *out = false;

        return true;
    }

    return false;
}

/**
 * Read a boolean JSON key.
 *
 * Optional boolean fields use this wrapper when absence should keep a default
 * model value.
 */
bool json_bool_key(const char* json, const char* key, const bool fallback)
{
    bool value;

    if (!json_bool_key_checked(json, key, &value))
        return fallback;

    return value;
}

/**
 * Read and validate a string JSON key.
 *
 * Model strings are currently simple ASCII tokens. Escaped strings are rejected
 * rather than partially decoded so unsupported input fails predictably.
 */
bool json_string_key_checked(const char* json, const char* key, char* out, const size_t out_len)
{
    const char* p = json_find_key(json, key);

    if (!p || out_len == 0)
        return false;

    p = json_after_colon(p);

    if (!p || *p != '"')
        return false;

    p++;

    const char* end = strchr(p, '"');

    if (!end)
        return false;

    for (const char* q = p; q < end; q++)
    {
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

/**
 * Read a string JSON key.
 *
 * Callers that already initialized a destination can ignore the boolean result
 * when an absent key should leave the previous value intact.
 */
void json_string_key(const char* json, const char* key, char* out, const size_t out_len)
{
    (void)json_string_key_checked(json, key, out, out_len);
}
