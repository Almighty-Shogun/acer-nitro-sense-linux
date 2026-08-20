#ifndef ANS_CLIENT_PARSE_H
#define ANS_CLIENT_PARSE_H

#include <stdbool.h>

bool client_parse_byte_value(const char *text, int *value);
bool client_parse_percent(const char *text, int *percent);

#endif
