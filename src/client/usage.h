#ifndef ANS_CLIENT_USAGE_H
#define ANS_CLIENT_USAGE_H

#include <stdio.h>

/**
 * Print command-line usage.
 *
 * Command handlers call this when argument shape validation fails.
 */
void client_usage(FILE* out);

#endif
