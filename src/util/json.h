#ifndef ANS_UTIL_JSON_H
#define ANS_UTIL_JSON_H

#include <stdbool.h>
#include <stddef.h>

#include "util/format.h"

int json_append_string(text_buffer *out, const char *value);
const char *json_find_key(const char *json, const char *key);
const char *json_after_colon(const char *p);
const char *json_find_array(const char *json, const char *key, const char **end_out);
const char *json_next_object(const char *p, const char *end, const char **obj_end);
void json_copy_slice(const char *start, const char *end, char *out, size_t out_len);
bool json_int_key_checked(const char *json, const char *key, int *out);
int json_int_key(const char *json, const char *key, int fallback);
bool json_bool_key_checked(const char *json, const char *key, bool *out);
bool json_bool_key(const char *json, const char *key, bool fallback);
bool json_string_key_checked(const char *json, const char *key, char *out, size_t out_len);
void json_string_key(const char *json, const char *key, char *out, size_t out_len);
const char *json_object_with_id(const char *json, const char *id);

#endif
