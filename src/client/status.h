#ifndef ANS_CLIENT_STATUS_H
#define ANS_CLIENT_STATUS_H

#include <stdbool.h>

enum temp_unit {
    TEMP_UNIT_CELSIUS,
    TEMP_UNIT_FAHRENHEIT,
};

int client_print_status(enum temp_unit unit, bool json);
int client_print_coolboost_status(void);

#endif
