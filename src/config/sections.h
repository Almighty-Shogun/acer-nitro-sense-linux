#ifndef ANS_CONFIG_SECTIONS_H
#define ANS_CONFIG_SECTIONS_H

#include "config/types.h"

/**
 * Parse allowed DMI model names.
 *
 * The resulting allow-list prevents a model profile from being applied to
 * unrelated hardware by default.
 */
int config_parse_allowed_dmi(const char* json, struct ans_config* cfg);

/**
 * Parse EC initialization writes.
 *
 * Init writes prepare model-specific EC state before fan control starts.
 */
int config_parse_init_writes(const char* json, struct ans_config* cfg);

/**
 * Parse fan definitions and curves.
 *
 * Fan parsing validates EC registers, sensor groups, write ranges, and curve
 * thresholds for every controllable component.
 */
int config_parse_fans(const char* json, struct ans_config* cfg);

/**
 * Parse named fan presets.
 *
 * Presets are optional user-facing shortcuts for applying CPU and GPU fan
 * percentages together.
 */
int config_parse_presets(const char* json, struct ans_config* cfg);

/**
 * Parse fan safety policy.
 *
 * Safety values control critical-temperature handling, fallback speeds, and EC
 * failure thresholds.
 */
int config_parse_safety(const char* json, struct ans_config* cfg);

/**
 * Parse firmware fan-mode registers.
 *
 * Fan modes enable firmware-auto and CoolBoost behavior when the model exposes
 * the required EC registers.
 */
int config_parse_fan_modes(const char* json, struct ans_config* cfg);

/**
 * Parse platform profile mappings.
 *
 * Platform profiles map names such as quiet and performance to firmware EC
 * values.
 */
int config_parse_platform_profiles(const char* json, struct ans_config* cfg);

/**
 * Parse AC and battery profile policy.
 *
 * These settings tell the daemon which platform profile to apply for each
 * power source.
 */
int config_parse_power_source_profiles(const char* json, struct ans_config* cfg);

/**
 * Parse keyboard backlight capabilities.
 *
 * Keyboard backlight parsing records EC brightness registers and optional
 * daemon-managed timeout behavior.
 */
int config_parse_keyboard_backlight(const char* json, struct ans_config* cfg);

#endif
