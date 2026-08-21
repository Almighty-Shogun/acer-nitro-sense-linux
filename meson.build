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
  'src/commands/parser.c',
  'src/commands/tokens.c',
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
  'src/commands/daemon.c',
  'src/daemon/diagnostics.c',
  'src/daemon/feature_status.c',
  'src/daemon/loop.c',
  'src/commands/readonly.c',
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
  'src/commands/coolboost.c',
  'src/commands/ec.c',
  'src/commands/fan.c',
  'src/commands/fan_mode.c',
  'src/commands/gpu_temp.c',
  'src/fan/control.c',
  'src/fan/lifecycle.c',
  'src/fan/observation.c',
  'src/fan/safety.c',
  'src/fan/temperature_cache.c',
  'src/fan/temperature_filter.c',
]

platform_sources = [
  'src/commands/keyboard_backlight.c',
  'src/commands/platform.c',
  'src/commands/power_source.c',
  'src/commands/profile.c',
  'src/keyboard/backlight.c',
  'src/keyboard/backlight_timeout.c',
  'src/platform/control.c',
  'src/platform/fan_mode.c',
  'src/platform/profile.c',
  'src/platform/sensor_power.c',
]

selftest_sources = [
  'tests/selftest/command_ec.c',
  'tests/selftest/command_fan.c',
  'tests/selftest/command_firmware.c',
  'tests/selftest/command_invalid.c',
  'tests/selftest/commands.c',
  'tests/selftest/platform_fan_mode.c',
  'tests/selftest/platform_keyboard_backlight.c',
  'tests/selftest/platform_power.c',
  'tests/selftest/platform_commands.c',
  'tests/selftest/platform_status.c',
  'tests/selftest/daemon_args.c',
  'tests/selftest/state.c',
  'tests/selftest/daemon.c',
  'tests/selftest/fan_auto.c',
  'tests/selftest/fan_control.c',
  'tests/selftest/fan_ec.c',
  'tests/selftest/fan_resume.c',
  'tests/selftest/fan_safety.c',
  'tests/selftest/fan_temperature.c',
  'tests/selftest/parser.c',
  'tests/selftest/helpers.c',
  'tests/selftest/fixture.c',
]

daemon_sources = core_sources + fan_sources + platform_sources

executable('acer-nitro-sensed',
  ['src/daemon.c'] + daemon_sources,
  include_directories: src_inc,
  dependencies: dl,
  install: true)

selftest = executable('acer-nitro-sense-selftest',
  ['tests/selftest/main.c'] + daemon_sources + selftest_sources,
  include_directories: [src_inc, tests_inc],
  dependencies: dl,
  install: false)

executable('acer-nitro-sense',
  [
    'src/client.c',
    'src/commands/client_dispatch.c',
    'src/client/doctor.c',
    'src/client/doctor_util.c',
    'src/commands/client_fan.c',
    'src/client/parse.c',
    'src/commands/client_platform.c',
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

test('daemon-self-test',
  selftest)

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
