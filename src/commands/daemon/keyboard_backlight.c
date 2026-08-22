#include "commands/daemon/platform_handlers.h"

#include "daemon/state.h"
#include "control/protocol.h"
#include "keyboard/backlight.h"
#include "commands/parser/parser.h"
#include "keyboard/backlight_timeout.h"

#include <stdio.h>
#include <string.h>

/**
 * Handles one mutating keyboard backlight subcommand.
 *
 * Brightness and timeout changes both need daemon runtime state, but they use
 * different command parsers and hardware paths.
 */
typedef bool (*keyboard_backlight_command_handler)(
    int client,
    struct ec_device* ec,
    const struct ans_config* cfg,
    const fan_state states[ANS_MAX_FANS],
    bool auto_mode,
    const char* preset,
    bool coolboost_enabled,
    daemon_runtime_state* runtime,
    const char* cmd
);

/**
 * Maps a keyboard backlight subcommand name to its daemon handler.
 *
 * Read-only status is handled by the registry before permission checks. This
 * table contains only actions that may alter state.
 */
typedef struct
{
    const char* name;
    keyboard_backlight_command_handler handler;
} keyboard_backlight_command;

/**
 * Describes one keyboard backlight timeout toggle value.
 *
 * The control socket uses readable on/off tokens while runtime state stores a
 * boolean flag.
 */
typedef struct
{
    const char* name;
    bool enabled;
} keyboard_backlight_timeout_value;

/**
 * Supported keyboard backlight timeout toggle values.
 *
 * The daemon persists this as a boolean but the control protocol exposes
 * readable on/off values.
 */
static const keyboard_backlight_timeout_value KEYBOARD_BACKLIGHT_TIMEOUT_VALUES[] = {
    {.name = "on", .enabled = true},
    {.name = "off", .enabled = false},
};

/**
 * Find the timeout toggle matching a parsed command token.
 *
 * Unknown values return NULL so timeout handling can share one usage error for
 * invalid toggles.
 */
static const keyboard_backlight_timeout_value* find_keyboard_backlight_timeout_value(const char* name)
{
    for (size_t i = 0; i < COMMAND_ARRAY_LEN(KEYBOARD_BACKLIGHT_TIMEOUT_VALUES); i++)
    {
        if (strcmp(name, KEYBOARD_BACKLIGHT_TIMEOUT_VALUES[i].name) == 0)
            return &KEYBOARD_BACKLIGHT_TIMEOUT_VALUES[i];
    }

    return NULL;
}

/**
 * Toggle the keyboard backlight idle timeout.
 *
 * Keyboard commands are optional because not every Nitro model exposes the
 * same EC registers. The handler reports unsupported hardware explicitly
 * instead of silently ignoring writes.
 */
static bool handle_keyboard_backlight_timeout_command(
    const int client,
    struct ec_device* ec,
    const struct ans_config* cfg,
    const fan_state states[ANS_MAX_FANS],
    const bool auto_mode,
    const char* preset,
    const bool coolboost_enabled,
    daemon_runtime_state* runtime,
    const char* cmd
)
{
    char action[16];

    (void)ec;

    if (!parse_keyboard_backlight_timeout_command(cmd, action, sizeof(action)))
    {
        control_reply(client, "error usage: keyboard-backlight timeout status|on|off\n");

        return true;
    }

    if (!cfg->keyboard_backlight.timeout_supported)
    {
        control_reply(client, "error keyboard-backlight timeout unavailable for this model\n");

        return true;
    }

    const keyboard_backlight_timeout_value* entry = find_keyboard_backlight_timeout_value(action);

    if (!entry)
    {
        control_reply(client, "error usage: keyboard-backlight timeout status|on|off\n");

        return true;
    }

    runtime->keyboard_backlight_timeout_enabled = entry->enabled;

    if (!runtime->keyboard_backlight_timeout_enabled)
        runtime->keyboard_backlight_timed_off = false;

    write_control_state(cfg, states, auto_mode, preset, coolboost_enabled, runtime);

    control_reply(
        client,
        "keyboard_backlight_timeout=%s timeout_seconds=%d backend=input-activity\n",
        runtime->keyboard_backlight_timeout_enabled ? "on" : "off",
        cfg->keyboard_backlight.timeout_seconds
    );

    return true;
}

/**
 * Set keyboard backlight brightness.
 *
 * Keyboard commands are optional because not every Nitro model exposes the
 * same EC registers. The handler reports unsupported hardware explicitly
 * instead of silently ignoring writes.
 */
static bool handle_keyboard_backlight_set_command(
    const int client,
    struct ec_device* ec,
    const struct ans_config* cfg,
    const fan_state states[ANS_MAX_FANS],
    const bool auto_mode,
    const char* preset,
    const bool coolboost_enabled,
    daemon_runtime_state* runtime,
    const char* cmd
)
{
    static const int brightness_steps[] = {0, 25, 50, 75, 100};

    int percent;
    struct keyboard_backlight_status status;

    (void)states;
    (void)auto_mode;
    (void)preset;
    (void)coolboost_enabled;

    if (!parse_keyboard_backlight_set_command(cmd, &percent))
    {
        control_reply(client, "error usage: keyboard-backlight set 0|25|50|75|100\n");

        return true;
    }

    if (!command_int_in(percent, brightness_steps, COMMAND_ARRAY_LEN(brightness_steps)))
    {
        control_reply(client, "error usage: keyboard-backlight set 0|25|50|75|100\n");

        return true;
    }

    if (!cfg->keyboard_backlight.available)
    {
        control_reply(client, "error keyboard-backlight unavailable: no EC backend configured for this model\n");

        return true;
    }

    if (!keyboard_backlight_set_percent(ec, cfg, percent, &status))
    {
        control_reply(
            client,
            "error keyboard-backlight write failed register=0x%02x percent=%d\n",
            cfg->keyboard_backlight.reg,
            percent
        );

        return true;
    }

    if (!daemon_quiet_logs)
        fprintf(
            stderr,
            "keyboard_backlight_change backend=ec register=0x%02x brightness=%d percent=%d\n",
            cfg->keyboard_backlight.reg,
            status.brightness,
            status.percent
        );

    keyboard_backlight_timeout_note_manual_set(runtime, status.percent);

    control_reply(
        client,
        "keyboard_backlight=available backend=ec register=0x%02x brightness=%d max_brightness=%d percent=%d\n",
        cfg->keyboard_backlight.reg,
        status.brightness,
        status.max_brightness,
        status.percent
    );

    return true;
}

/**
 * Supported mutating keyboard backlight commands.
 *
 * Status commands are handled before permission checks. This table only routes
 * brightness and timeout writes.
 */
static const keyboard_backlight_command KEYBOARD_BACKLIGHT_COMMANDS[] = {
    {.name = "set", .handler = handle_keyboard_backlight_set_command},
    {.name = "timeout", .handler = handle_keyboard_backlight_timeout_command},
};

/**
 * Find a keyboard backlight subcommand entry.
 *
 * Status is handled by the daemon registry before permission checks; mutating
 * actions route through this local table.
 */
static const keyboard_backlight_command* find_keyboard_backlight_command(const char* name)
{
    for (size_t i = 0; i < COMMAND_ARRAY_LEN(KEYBOARD_BACKLIGHT_COMMANDS); i++)
    {
        if (strcmp(name, KEYBOARD_BACKLIGHT_COMMANDS[i].name) == 0)
            return &KEYBOARD_BACKLIGHT_COMMANDS[i];
    }

    return NULL;
}

/**
 * Dispatch a keyboard backlight command.
 *
 * The dispatcher validates the subcommand shape before sending brightness or
 * timeout changes to the matching hardware-specific handler.
 */
bool handle_keyboard_backlight_command(
    const int client,
    struct ec_device* ec,
    const struct ans_config* cfg,
    const fan_state states[ANS_MAX_FANS],
    const bool auto_mode,
    const char* preset,
    const bool coolboost_enabled,
    daemon_runtime_state* runtime,
    const char* cmd
)
{
    char action[16];

    if (!command_second_token(cmd, action, sizeof(action)))
    {
        control_reply(client, "error usage: keyboard-backlight status|set 0|25|50|75|100|timeout status|on|off\n");

        return true;
    }

    const keyboard_backlight_command* entry = find_keyboard_backlight_command(action);

    if (!entry)
    {
        control_reply(client, "error usage: keyboard-backlight status|set 0|25|50|75|100|timeout status|on|off\n");

        return true;
    }

    return entry->handler(client, ec, cfg, states, auto_mode, preset, coolboost_enabled, runtime, cmd);
}
