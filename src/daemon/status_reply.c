#include "daemon/status.h"

#include "ec/ec.h"
#include "control/protocol.h"
#include "platform/control.h"
#include "daemon/status_format.h"
#include "platform/power_source.h"
#include "daemon/diagnostics_formats.h"

/**
 * Write status.
 *
 * Status replies are the daemon-facing representation of current hardware and
 * control state. They keep human CLI output and machine-readable state in
 * sync.
 */
void reply_status(
    const int client,
    const struct ans_config* cfg,
    const fan_state states[ANS_MAX_FANS],
    const bool auto_mode,
    const char* preset,
    const bool coolboost_enabled
)
{
    const bool firmware_mode = firmware_auto_mode(auto_mode, preset);
    const enum power_source_state power_source = read_power_source();

    control_reply(
        client,
        STATUS_REPLY_FORMAT_MODEL,
        cfg->model,
        control_mode(auto_mode, preset),
        auto_mode ? 1 : 0, preset,
        cfg->fan_modes.available ? (coolboost_enabled ? "on" : "off") : "unavailable",
        power_source_name(power_source)
    );

    for (int i = 0; i < cfg->fan_len; i++)
    {
        char active_percent[16];

        const char* control = status_fan_control_source(firmware_mode, states[i].safety_active);

        status_active_percent_text(
            active_percent,
            sizeof(active_percent),
            firmware_mode,
            states[i].safety_active,
            states[i].percent,
            "firmware"
        );

        control_reply(
            client,
            STATUS_REPLY_FORMAT_FAN,
            cfg->fans[i].id, states[i].rpm, states[i].temp_c,
            control, active_percent, status_requested_percent(&states[i]),
            states[i].percent,
            states[i].percent, states[i].write_value,
            states[i].safety_active ? "active" : "ok",
            states[i].safety_active ? " reason=" : "",
            states[i].safety_active ? states[i].safety_reason : "",
            states[i].critical_temp_samples, states[i].ec_read_failures,
            states[i].ec_write_failures
        );
    }
}

/**
 * Write preset show.
 *
 * Status replies are the daemon-facing representation of current hardware and
 * control state. They keep human CLI output and machine-readable state in
 * sync.
 */
void reply_preset_show(
    const int client,
    const struct ans_config* cfg,
    const fan_state states[ANS_MAX_FANS],
    const bool auto_mode,
    const char* preset
)
{
    control_reply(client, "mode=%s preset=%s\n", control_mode(auto_mode, preset), preset);

    for (int i = 0; i < cfg->fan_len; i++)
    {
        const int requested = status_requested_percent(&states[i]);

        control_reply(client, "%s requested=%d effective=%d percent=%d\n", cfg->fans[i].id, requested, states[i].percent, requested);
    }
}

/**
 * Write presets.
 *
 * Status replies are the daemon-facing representation of current hardware and
 * control state. They keep human CLI output and machine-readable state in
 * sync.
 */
void reply_presets(const int client, const struct ans_config* cfg)
{
    for (int i = 0; i < cfg->preset_len; i++)
        control_reply(client, "preset=%s cpu=%d gpu=%d\n", cfg->presets[i].id, cfg->presets[i].cpu, cfg->presets[i].gpu);
}

/**
 * Write read.
 *
 * Status replies are the daemon-facing representation of current hardware and
 * control state. They keep human CLI output and machine-readable state in
 * sync.
 */
void reply_ec_read(const int client, struct ec_device* ec, const int reg)
{
    const int value = ec_read_byte(ec, reg);

    if (value < 0)
    {
        control_reply(client, "error ec read failed register=0x%02x\n", reg);

        return;
    }

    control_reply(client, "ec[0x%02x]=0x%02x (%d)\n", reg, value, value);
}

/**
 * Write dump.
 *
 * Status replies are the daemon-facing representation of current hardware and
 * control state. They keep human CLI output and machine-readable state in
 * sync.
 */
void reply_ec_dump(const int client, struct ec_device* ec, const int start, const int end)
{
    for (int base = start; base <= end; base += 16)
    {
        const int row_end = base + 15 < end ? base + 15 : end;

        control_reply(client, "0x%02x:", base);

        for (int reg = base; reg <= row_end; reg++)
        {
            const int value = ec_read_byte(ec, reg);

            if (value < 0)
            {
                control_reply(client, " ??");
            }
            else
            {
                control_reply(client, " %02x", value);
            }
        }

        control_reply(client, "\n");
    }
}
