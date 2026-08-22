#ifndef ANS_UTIL_JSON_H
#define ANS_UTIL_JSON_H

#include <stddef.h>
#include <stdbool.h>

#include "util/format.h"

/**
 * Append a JSON-escaped string.
 *
 * The value is quoted and escaped into the caller's bounded text buffer.
 */
int json_append_string(text_buffer* out, const char* value);

/**
 * Find a top-level JSON key.
 *
 * The parser is intentionally small and profile-focused, so callers use this
 * helper before reading the specific value type they expect.
 */
const char* json_find_key(const char* json, const char* key);

/**
 * Move a JSON cursor past a key separator.
 *
 * NULL is returned when the cursor is not positioned before a colon separator.
 */
const char* json_after_colon(const char* p);

/**
 * Find a JSON array by key.
 *
 * The returned start and end pointers delimit the array contents without
 * allocating a copy.
 */
const char* json_find_array(const char* json, const char* key, const char** end_out);

/**
 * Return the next object from a JSON array.
 *
 * The helper advances through object slices so section parsers can process
 * arrays without a full JSON DOM.
 */
const char* json_next_object(const char* p, const char* end, const char** obj_end);

/**
 * Copy a JSON source slice into a bounded buffer.
 *
 * The destination is always terminated when the buffer has space.
 */
void json_copy_slice(const char* start, const char* end, char* out, size_t out_len);

/**
 * Read and validate an integer JSON key.
 *
 * The function returns false when the key is missing or the value is not a
 * complete integer.
 */
bool json_int_key_checked(const char* json, const char* key, int* out);

/**
 * Read an integer JSON key.
 *
 * Missing or invalid values return the caller-supplied fallback.
 */
int json_int_key(const char* json, const char* key, int fallback);

/**
 * Read and validate a boolean JSON key.
 *
 * Only JSON boolean literals are accepted as valid values.
 */
bool json_bool_key_checked(const char* json, const char* key, bool* out);

/**
 * Read a boolean JSON key.
 *
 * Missing or invalid values return the caller-supplied fallback.
 */
bool json_bool_key(const char* json, const char* key, bool fallback);

/**
 * Read and validate a string JSON key.
 *
 * The destination buffer must hold the complete decoded string for the read to
 * succeed.
 */
bool json_string_key_checked(const char* json, const char* key, char* out, size_t out_len);

/**
 * Read a string JSON key.
 *
 * Missing or invalid values leave the destination as an empty string.
 */
void json_string_key(const char* json, const char* key, char* out, size_t out_len);

/**
 * Find a JSON object by id.
 *
 * Model profile sections use this to find named objects without building an
 * intermediate representation.
 */
const char* json_object_with_id(const char* json, const char* id);

#endif
