#include "daemon/status_json_internal.h"

#include "util/json.h"
#include "daemon/status_format.h"

/**
 * Append fan status JSON.
 *
 * Status JSON is consumed by automation and the GNOME extension, so field
 * names and availability flags need to remain stable across releases.
 */
void append_fan_status_json(
    text_buffer* out, const struct ans_config* cfg,
    const fan_state states[ANS_MAX_FANS],
    const hardware_names* names,
    const bool firmware_mode,
    const int index
)
{
    char active_percent[16];

    const char* control = status_fan_control_source(firmware_mode, states[index].safety_active);

    status_active_percent_text(
        active_percent,
        sizeof(active_percent),
        firmware_mode,
        states[index].safety_active,
        states[index].percent,
        "null"
    );

    text_buffer_append(out, "    { \"id\": ");
    json_append_string(out, cfg->fans[index].id);
    text_buffer_append(out, ", \"name\": ");
    json_append_string(out, cfg->fans[index].name);
    text_buffer_append(out, ", \"component_name\": ");
    json_append_string(out, component_name_for_fan(names, &cfg->fans[index]));

    text_buffer_append(
        out,
        ", \"rpm\": %d, \"temp_c\": %d, \"sensor_temp_c\": %d, "
        "\"control_temp_c\": %d, \"control_sensor_temp_c\": %d, "
        "\"temp_available\": %s, \"control_temp_available\": %s, "
        "\"control\": ",
        states[index].rpm, states[index].temp_c, states[index].sensor_temp_c,
        states[index].control_temp_c, states[index].control_sensor_temp_c,
        bool_text(states[index].temp_available),
        bool_text(states[index].control_temp_available)
    );

    json_append_string(out, control);

    text_buffer_append(
        out,
        ", \"active_percent\": %s, "
        "\"firmware_controlled\": %s, \"percent\": %d, "
        "\"requested_percent\": %d, \"effective_percent\": %d, "
        "\"write_value\": %d, "
        "\"critical_temp_samples\": %d, "
        "\"ec_read_failures\": %d, \"ec_write_failures\": %d, "
        "\"safety_active\": %s, "
        "\"safety_reason\": ",
        active_percent,
        bool_text(status_fan_firmware_controlled(firmware_mode, states[index].safety_active)),
        states[index].percent,
        status_requested_percent(&states[index]),
        states[index].percent,
        states[index].write_value,
        states[index].critical_temp_samples,
        states[index].ec_read_failures, states[index].ec_write_failures,
        bool_text(states[index].safety_active)
    );

    json_append_string(out, states[index].safety_reason);
    text_buffer_append(out, " }%s\n", index == cfg->fan_len - 1 ? "" : ",");
}
