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

subdir('src')
subdir('tests')

executable('acer-nitro-sensed',
  ['src/daemon.c'] + daemon_sources,
  include_directories: src_inc,
  dependencies: dl,
  install: true)

selftest = executable('acer-nitro-sense-selftest',
  selftest_main + daemon_sources + selftest_sources,
  include_directories: [src_inc, tests_inc],
  dependencies: dl,
  install: false)

executable('acer-nitro-sense',
  client_sources,
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
