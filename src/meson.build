command_parser_sources = files(
  'commands/parser/parser.c',
  'commands/parser/tokens.c',
)

config_sources = files(
  'config/config.c',
  'config/fans.c',
  'config/lists.c',
  'config/platform.c',
  'config/parse.c',
  'config/safety.c',
  'config/sections.c',
)

control_sources = files(
  'control/protocol.c',
  'control/socket.c',
)

daemon_core_sources = files(
  'daemon/args.c',
  'daemon/capabilities.c',
  'daemon/diagnostics.c',
  'daemon/diagnostics_error.c',
  'daemon/diagnostics_validate.c',
  'daemon/feature_status.c',
  'daemon/loop.c',
  'daemon/state_persist.c',
  'daemon/state_restore.c',
  'daemon/status.c',
  'daemon/status_json.c',
  'daemon/status_json_fans.c',
  'daemon/status_reply.c',
)

daemon_command_sources = files(
  'commands/daemon/control.c',
  'commands/daemon/coolboost.c',
  'commands/daemon/daemon.c',
  'commands/daemon/ec.c',
  'commands/daemon/fan.c',
  'commands/daemon/fan_context.c',
  'commands/daemon/fan_mode.c',
  'commands/daemon/gpu_temp.c',
  'commands/daemon/keyboard_backlight.c',
  'commands/daemon/power_source.c',
  'commands/daemon/profile.c',
  'commands/daemon/registry.c',
)

ec_sources = files(
  'ec/ec.c',
  'ec/fake.c',
  'ec/file.c',
  'ec/io_ports.c',
)

fan_sources = files(
  'fan/control.c',
  'fan/lifecycle.c',
  'fan/observation.c',
  'fan/safety.c',
  'fan/safety_log.c',
  'fan/safety_policy.c',
  'fan/temperature_cache.c',
  'fan/temperature_filter.c',
)

hardware_sources = files(
  'hardware/cpu.c',
  'hardware/dmi.c',
  'hardware/gpu.c',
  'hardware/hardware.c',
)

platform_sources = files(
  'input/activity.c',
  'keyboard/backlight.c',
  'keyboard/backlight_ec.c',
  'keyboard/backlight_sysfs.c',
  'keyboard/backlight_timeout.c',
  'platform/control.c',
  'platform/fan_mode.c',
  'platform/power_source.c',
  'platform/profile.c',
  'platform/sensor_power.c',
)

sensor_sources = files(
  'sensors/fake.c',
  'sensors/nvidia.c',
  'sensors/power.c',
  'sensors/power_paths.c',
  'sensors/sensors.c',
)

shared_util_sources = files(
  'util/fd.c',
  'util/file.c',
  'util/number.c',
  'util/process.c',
  'util/string.c',
)

daemon_util_sources = files(
  'util/format.c',
  'util/json.c',
  'util/json_scan.c',
)

daemon_sources = command_parser_sources
daemon_sources += config_sources
daemon_sources += control_sources
daemon_sources += daemon_core_sources
daemon_sources += daemon_command_sources
daemon_sources += ec_sources
daemon_sources += fan_sources
daemon_sources += hardware_sources
daemon_sources += platform_sources
daemon_sources += sensor_sources
daemon_sources += shared_util_sources
daemon_sources += daemon_util_sources

client_command_sources = files(
  'commands/client/builtin.c',
  'commands/client/dispatch.c',
  'commands/client/fan.c',
  'commands/client/platform.c',
)

client_core_sources = files(
  'client.c',
  'client/doctor.c',
  'client/doctor_exec.c',
  'client/doctor_os.c',
  'client/doctor_permissions.c',
  'client/doctor_probes.c',
  'client/doctor_util.c',
  'client/parse.c',
  'client/service.c',
  'client/status.c',
  'client/transport.c',
  'client/usage.c',
)

client_sources = client_core_sources + client_command_sources + command_parser_sources + shared_util_sources
