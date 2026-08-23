#ifndef ANS_DAEMON_ARGS_H
#define ANS_DAEMON_ARGS_H

#include <stdbool.h>

/**
 * Parsed daemon startup options.
 *
 * These flags decide whether the daemon starts normally, validates a config,
 * probes EC registers, or bypasses DMI model checks for diagnostics.
 */
struct daemon_args
{
    const char* config_path;
    bool config_path_explicit;
    bool force_model;
    bool check_config;
    bool probe;
    bool validate;
};

/**
 * Initialize daemon arguments to their default values.
 *
 * Defaults represent the normal installed service path with no diagnostic
 * mode enabled.
 */
void daemon_args_init(struct daemon_args* args);

/**
 * Parse daemon CLI arguments into startup options.
 *
 * The parser keeps service startup, validation, and probe flags in one
 * structure so main can select the daemon mode once.
 */
int daemon_args_parse(int argc, char* const argv[], struct daemon_args* args);

#endif
