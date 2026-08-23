#include "json.h"

#include <stdio.h>
#include <string.h>

/**
 * Find a quoted key in a small JSON object.
 *
 * Model parsing only needs known top-level keys, so a simple quoted-key search
 * keeps the parser dependency-free without pretending to be a full JSON engine.
 */
const char* json_find_key(const char* json, const char* key)
{
    char needle[96];

    snprintf(needle, sizeof(needle), "\"%s\"", key);

    return strstr(json, needle);
}

/**
 * Move a JSON cursor past a key separator.
 *
 * Callers pass a pointer at or before a key's colon. The returned pointer is the
 * first non-whitespace byte of the value.
 */
const char* json_after_colon(const char* p)
{
    p = strchr(p, ':');

    if (!p)
        return NULL;

    p++;

    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;

    return p;
}

/**
 * Find an array value and return its inner slice.
 *
 * The returned start points after `[`, while `end_out` points at the matching
 * `]`. Nested arrays are counted so list parsers can scan object entries safely.
 */
const char* json_find_array(const char* json, const char* key, const char** end_out)
{
    int depth = 0;

    const char* p = json_find_key(json, key);

    if (!p || !end_out)
        return NULL;

    p = json_after_colon(p);

    if (!p || *p != '[')
        return NULL;

    for (const char* q = p; *q; q++)
    {
        if (*q == '[')
        {
            depth++;
        }
        else if (*q == ']')
        {
            depth--;

            if (depth == 0)
            {
                *end_out = q;

                return p + 1;
            }
        }
    }

    return NULL;
}

/**
 * Return the next object from a JSON array.
 *
 * Array parsers repeatedly call this with the previous object end to walk model
 * sections without allocating intermediate lists.
 */
const char* json_next_object(const char* p, const char* end, const char** obj_end)
{
    int depth = 0;
    const char* start = NULL;

    if (!p || !end || !obj_end)
        return NULL;

    for (; p < end && *p; p++)
    {
        if (*p == '{')
        {
            if (depth == 0)
                start = p;

            depth++;
        }
        else if (*p == '}')
        {
            depth--;

            if (depth == 0 && start)
            {
                *obj_end = p + 1;

                return start;
            }
        }
    }

    return NULL;
}

/**
 * Copy a JSON slice into a bounded string buffer.
 *
 * Section parsers use this to isolate one object before reading keys from it.
 * Invalid slices produce an empty string rather than leaking stale buffer data.
 */
void json_copy_slice(const char* start, const char* end, char* out, const size_t out_len)
{
    if (!out || out_len == 0) return;

    if (!start || !end || end < start)
    {
        out[0] = '\0';

        return;
    }

    size_t len = (size_t)(end - start);

    if (len >= out_len)
        len = out_len - 1;

    memcpy(out, start, len);

    out[len] = '\0';
}

/**
 * Find a JSON object by id.
 *
 * Model lookup currently uses stable object identifiers, so this helper keeps
 * that search in one place until a full parser is warranted.
 */
const char* json_object_with_id(const char* json, const char* id)
{
    char needle[64];

    snprintf(needle, sizeof(needle), "\"id\": \"%s\"", id);

    return strstr(json, needle);
}
