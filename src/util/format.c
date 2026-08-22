#include "util/format.h"

#include <stdio.h>
#include <stdarg.h>

/**
 * Initialize a bounded text buffer.
 *
 * The caller supplies storage, and the buffer starts as an empty string so it
 * can be passed directly to append helpers.
 */
void text_buffer_init(text_buffer* buf, char* data, const size_t cap)
{
    buf->data = data;
    buf->len = 0;
    buf->cap = cap;
    buf->truncated = false;

    if (cap > 0)
        data[0] = '\0';
}

/**
 * Append formatted text to a bounded buffer.
 *
 * Truncation is recorded on the buffer and reported as a failure, allowing
 * callers to keep composing output while still detecting incomplete replies.
 */
int text_buffer_append(text_buffer* buf, const char* fmt, ...)
{
    va_list ap;

    if (buf->len >= buf->cap)
    {
        buf->truncated = true;

        return -1;
    }

    va_start(ap, fmt);
    const int written = vsnprintf(buf->data + buf->len, buf->cap - buf->len, fmt, ap);
    va_end(ap);

    if (written < 0)
    {
        buf->truncated = true;

        return -1;
    }

    if ((size_t)written >= buf->cap - buf->len)
    {
        buf->len = buf->cap - 1;
        buf->truncated = true;

        return -1;
    }

    buf->len += (size_t)written;

    return 0;
}

/**
 * Return whether the text buffer is still writable.
 *
 * This summarizes every previous append attempt, which is easier for callers
 * than checking each individual append when building long status payloads.
 */
bool text_buffer_ok(const text_buffer* buf)
{
    return !buf->truncated;
}

/**
 * Format a boolean as status text.
 *
 * Daemon replies use stable textual booleans instead of exposing C integer
 * values.
 */
const char* bool_text(const bool value)
{
    return value ? "true" : "false";
}

/**
 * Format availability as status text.
 *
 * This keeps feature status replies consistent across fan, platform, and
 * keyboard capabilities.
 */
const char* availability_text(const bool available)
{
    return available ? "available" : "unavailable";
}

/**
 * Format enabled state as on or off.
 *
 * User-facing command replies use on/off for toggles because it matches the
 * accepted CLI arguments.
 */
const char* on_off_text(const bool enabled)
{
    return enabled ? "on" : "off";
}

/**
 * Format support state as status text.
 *
 * Unsupported hardware paths report unsupported rather than unavailable so
 * diagnostics can distinguish missing features from inactive ones.
 */
const char* supported_text(const bool supported)
{
    return supported ? "available" : "unsupported";
}

/**
 * Choose a fallback status value.
 *
 * This avoids scattered NULL checks when optional Linux metadata is missing.
 */
const char* fallback_text(const char* text, const char* fallback)
{
    return text && text[0] ? text : fallback;
}
