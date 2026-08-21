project('acer-nitro-sense-linux', 'c',
  version: '0.1.0',
  default_options: ['c_std=c17', 'warning_level=2'])

prefix = get_option('prefix')
datadir = join_paths(prefix, get_option('datadir'))
systemd = dependency('systemd', required: false)
systemd_system_unit_dir = systemd.get_variable(
  pkgconfig: 'systemdsystemunitdir',
  default_value: join_paths(prefix, 'lib', 'systemd', 'system'))
systemd_sysusers_dir = systemd.get_variable(
  pkgconfig: 'sysusersdir',
  default_value: join_paths(prefix, 'lib', 'sysusers.d'))
systemd_system_sleep_dir = join_paths(prefix, 'lib', 'systemd', 'system-sleep')
cc = meson.get_compiler('c')
dl = cc.find_library('dl', required: false)
src_inc = include_directories('src')
tests_inc = include_directories('tests')

add_project_arguments('-D_GNU_SOURCE', language: 'c')

if get_option('dev_checks')
  add_project_arguments(
    cc.get_supported_arguments([
      '-Wformat=2',
      '-Wundef',
      '-Wshadow',
      '-Wstrict-prototypes',
      '-Wold-style-definition',
    ]),
    language: 'c')
endif

core_sources = [
  'src/commands/parser/parser.c',
  'src/commands/parser/tokens.c',
  'src/control/protocol.c',
  'src/config/config.c',
  'src/config/fans.c',
  'src/config/lists.c',
  'src/config/platform.c',
  'src/config/parse.c',
  'src/config/safety.c',
  'src/config/sections.c',
  'src/control/socket.c',
  'src/daemon/args.c',
  'src/daemon/capabilities.c',
  'src/commands/daemon/daemon.c',
  'src/daemon/diagnostics.c',
  'src/daemon/feature_status.c',
  'src/daemon/loop.c',
  'src/commands/daemon/readonly.c',
  'src/daemon/state.c',
  'src/daemon/status.c',
  'src/daemon/status_reply.c',
  'src/ec/ec.c',
  'src/ec/fake.c',
  'src/ec/file.c',
  'src/ec/io_ports.c',
  'src/hardware/hardware.c',
  'src/hardware/cpu.c',
  'src/hardware/dmi.c',
  'src/hardware/gpu.c',
  'src/input/activity.c',
  'src/platform/power_source.c',
  'src/sensors/sensors.c',
  'src/sensors/fake.c',
  'src/sensors/nvidia.c',
  'src/sensors/power.c',
  'src/util/fd.c',
  'src/util/file.c',
  'src/util/format.c',
  'src/util/json.c',
  'src/util/json_scan.c',
  'src/util/number.c',
  'src/util/string.c',
]

fan_sources = [
  'src/commands/daemon/coolboost.c',
  'src/commands/daemon/ec.c',
  'src/commands/daemon/fan.c',
  'src/commands/daemon/fan_mode.c',
  'src/commands/daemon/gpu_temp.c',
  'src/fan/control.c',
  'src/fan/lifecycle.c',
  'src/fan/observation.c',
  'src/fan/safety.c',
  'src/fan/temperature_cache.c',
  'src/fan/temperature_filter.c',
]

platform_sources = [
  'src/commands/daemon/keyboard_backlight.c',
  'src/commands/daemon/platform.c',
  'src/commands/daemon/power_source.c',
  'src/commands/daemon/profile.c',
  'src/keyboard/backlight.c',
  'src/keyboard/backlight_timeout.c',
  'src/platform/control.c',
  'src/platform/fan_mode.c',
  'src/platform/profile.c',
  'src/platform/sensor_power.c',
]

unit_sources = [
  'tests/unit/command_ec.c',
  'tests/unit/command_fan.c',
  'tests/unit/command_firmware.c',
  'tests/unit/command_invalid.c',
  'tests/unit/commands.c',
  'tests/unit/platform_fan_mode.c',
  'tests/unit/platform_keyboard_backlight.c',
  'tests/unit/platform_power.c',
  'tests/unit/platform_commands.c',
  'tests/unit/platform_status.c',
  'tests/unit/daemon_args.c',
  'tests/unit/state.c',
  'tests/unit/daemon.c',
  'tests/unit/fan_auto.c',
  'tests/unit/fan_control.c',
  'tests/unit/fan_ec.c',
  'tests/unit/fan_resume.c',
  'tests/unit/fan_safety.c',
  'tests/unit/fan_temperature.c',
  'tests/unit/parser.c',
  'tests/unit/helpers.c',
  'tests/unit/fixture.c',
  'tests/unit/utility.c',
]

daemon_sources = core_sources + fan_sources + platform_sources

executable('acer-nitro-sensed',
  ['src/daemon.c'] + daemon_sources,
  include_directories: src_inc,
  dependencies: dl,
  install: true)

unit = executable('acer-nitro-sense-unit-test',
  ['tests/unit/main.c'] + daemon_sources + unit_sources,
  include_directories: [src_inc, tests_inc],
  dependencies: dl,
  install: false)

executable('acer-nitro-sense',
  [
    'src/client.c',
    'src/commands/client/dispatch.c',
    'src/client/doctor.c',
    'src/client/doctor_util.c',
    'src/commands/client/fan.c',
    'src/client/parse.c',
    'src/commands/client/platform.c',
    'src/client/service.c',
    'src/client/status.c',
    'src/client/transport.c',
    'src/client/usage.c',
    'src/util/fd.c',
    'src/util/file.c',
    'src/util/number.c',
    'src/util/string.c',
  ],
  include_directories: src_inc,
  install: true)

install_symlink('ans',
  pointing_to: 'acer-nitro-sense',
  install_dir: join_paths(prefix, get_option('bindir')))

install_data('models/acer-nitro-an517-51.json',
  install_dir: join_paths(datadir, 'acer-nitro-sense', 'models'))

install_data('systemd/acer-nitro-sense.service',
  install_dir: systemd_system_unit_dir)

install_data('systemd/acer-nitro-sense.sysusers',
  rename: 'acer-nitro-sense.conf',
  install_dir: systemd_sysusers_dir)

install_data(
  'systemd/acer-nitro-sense.system-sleep',
  rename: 'acer-nitro-sense',
  install_mode: 'rwxr-xr-x',
  install_dir: systemd_system_sleep_dir)

test('daemon-unit-test',
  unit)

test('config-real-model',
  find_program('sh'),
  args: ['-c', '@0@/acer-nitro-sensed --config @1@/models/acer-nitro-an517-51.json --force-model --check-config'.format(meson.current_build_dir(), meson.current_source_dir())])

test('config-fake-model',
  find_program('sh'),
  args: ['-c', '@0@/acer-nitro-sensed --config @1@/tests/fixtures/fake-model.json --force-model --check-config'.format(meson.current_build_dir(), meson.current_source_dir())])

test('config-invalid-missing-fan-register',
  find_program('sh'),
  args: ['-c', '! @0@/acer-nitro-sensed --config @1@/tests/fixtures/invalid-missing-fan-register.json --force-model --check-config'.format(meson.current_build_dir(), meson.current_source_dir())])

test('config-invalid-malformed-number',
  find_program('sh'),
  args: ['-c', '! @0@/acer-nitro-sensed --config @1@/tests/fixtures/invalid-malformed-number.json --force-model --check-config'.format(meson.current_build_dir(), meson.current_source_dir())])

test('fake-ec-probe',
  find_program('sh'),
  args: ['-c', 'ANS_FAKE_CPU_TEMP_C=55 ANS_FAKE_GPU_TEMP_C=50 @0@/acer-nitro-sensed --config @1@/tests/fixtures/fake-model.json --force-model --probe | grep -q "backend=fake"'.format(meson.current_build_dir(), meson.current_source_dir())])

test('fake-model-validation',
  find_program('sh'),
  args: ['-c', 'ANS_FAKE_CPU_TEMP_C=55 ANS_FAKE_GPU_TEMP_C=50 @0@/acer-nitro-sensed --config @1@/tests/fixtures/fake-model.json --force-model --validate-model | grep -q "fan=cpu"'.format(meson.current_build_dir(), meson.current_source_dir())])

test('validate-summary-smoke',
  find_program('sh'),
  args: ['@0@/tests/validate-summary-smoke.sh'.format(meson.current_source_dir())])
