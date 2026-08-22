#ifndef ANS_CLIENT_STATUS_H
#define ANS_CLIENT_STATUS_H

#include <stdbool.h>

/**
 * Temperature unit used when formatting human-readable status output.
 *
 * The daemon reports temperatures in Celsius. The client converts only the
 * final presentation layer so protocol output stays stable.
 */
enum temp_unit
{
    TEMP_UNIT_CELSIUS,
    TEMP_UNIT_FAHRENHEIT,
};

/**
 * Print daemon status in the requested unit and format.
 *
 * Human-readable output supports unit conversion, while JSON output keeps the
 * daemon's raw Celsius values for machine consumers.
 */
int client_print_status(enum temp_unit unit, bool json);

/**
 * Print the current CoolBoost state.
 *
 * This helper queries the daemon and formats only the CoolBoost status line
 * instead of printing the full fan status payload.
 */
int client_print_coolboost_status(void);

#endif
