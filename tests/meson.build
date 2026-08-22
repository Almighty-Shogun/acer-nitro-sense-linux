command_unit_sources = files(
  'unit/commands/commands.c',
  'unit/commands/daemon.c',
  'unit/commands/ec.c',
  'unit/commands/fan.c',
  'unit/commands/firmware.c',
  'unit/commands/invalid.c',
)

platform_unit_sources = files(
  'unit/platform/commands.c',
  'unit/platform/fan_mode.c',
  'unit/platform/keyboard_backlight.c',
  'unit/platform/power.c',
  'unit/platform/status.c',
)

fan_unit_sources = files(
  'unit/fan/auto.c',
  'unit/fan/control.c',
  'unit/fan/ec.c',
  'unit/fan/resume.c',
  'unit/fan/safety.c',
  'unit/fan/temperature.c',
)

daemon_unit_sources = files(
  'unit/daemon/args.c',
  'unit/daemon/daemon.c',
  'unit/daemon/state.c',
  'unit/daemon/status_json.c',
)

parser_unit_sources = files(
  'unit/parser/parser.c',
)

core_unit_sources = files(
  'unit/fixture.c',
  'unit/helpers.c',
  'unit/utility.c',
)

unit_sources = command_unit_sources
unit_sources += platform_unit_sources
unit_sources += fan_unit_sources
unit_sources += daemon_unit_sources
unit_sources += parser_unit_sources
unit_sources += core_unit_sources

unit_main = files('unit/main.c')
