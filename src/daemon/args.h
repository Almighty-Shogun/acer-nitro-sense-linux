#ifndef ANS_DAEMON_ARGS_H
#define ANS_DAEMON_ARGS_H

#include <stdbool.h>

struct daemon_args {
    const char *config_path;
    bool config_path_explicit;
    bool force_model;
    bool check_config;
    bool probe;
    bool validate;
};

void daemon_args_init(struct daemon_args *args);
int daemon_args_parse(int argc, char *const argv[], struct daemon_args *args);

#endif
