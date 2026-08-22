#ifndef ANS_UTIL_FORMAT_H
#define ANS_UTIL_FORMAT_H

#include <stddef.h>
#include <stdbool.h>

/**
 * Fixed-size append-only formatter with truncation tracking.
 *
 * Status and diagnostic output use this buffer to build bounded strings
 * without losing whether the result was truncated.
 */
typedef struct text_buffer
{
    char* data;
    size_t len;
    size_t cap;
    bool truncated;
} text_buffer;

/**
 * Initialize a fixed-size text buffer.
 *
 * The buffer starts empty and records the caller-provided capacity so later
 * appends can fail safely.
 */
void text_buffer_init(text_buffer* buf, char* data, size_t cap);

/**
 * Append formatted text and track truncation.
 *
 * The return value follows printf-style formatting while the buffer itself
 * records whether any append exceeded capacity.
 */
int text_buffer_append(text_buffer* buf, const char* fmt, ...)
__attribute__((format(printf, 2, 3)));

/**
 * Return whether the text buffer is still writable.
 *
 * Callers use this after one or more appends to decide whether the produced
 * status payload is complete.
 */
bool text_buffer_ok(const text_buffer* buf);

/**
 * Format a boolean as status text.
 *
 * Daemon replies use stable textual booleans instead of exposing C integer
 * values.
 */
const char* bool_text(bool value);

/**
 * Format availability as status text.
 *
 * This keeps feature status replies consistent across fan, platform, and
 * keyboard capabilities.
 */
const char* availability_text(bool available);

/**
 * Format enabled state as on or off.
 *
 * User-facing command replies use on/off for toggles because it matches the
 * accepted CLI arguments.
 */
const char* on_off_text(bool enabled);

/**
 * Format support state as status text.
 *
 * Unsupported hardware paths report unsupported rather than unavailable so
 * diagnostics can distinguish missing features from inactive ones.
 */
const char* supported_text(bool supported);

/**
 * Choose a fallback status value.
 *
 * This avoids scattered NULL checks when optional Linux metadata is missing.
 */
const char* fallback_text(const char* text, const char* fallback);

#endif
