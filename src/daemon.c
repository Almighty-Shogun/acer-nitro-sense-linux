#include "commands/daemon.h"
#include "config/config.h"
#include "core/constants.h"
#include "control/socket.h"
#include "daemon/args.h"
#include "daemon/diagnostics.h"
#include "daemon/loop.h"
#include "daemon/state.h"
#include "ec/ec.h"
#include "fan/control.h"
#include "hardware/hardware.h"
#include "keyboard/backlight_timeout.h"
#include "platform/control.h"
#include "util/file.h"
#include "util/string.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

bool daemon_quiet_logs = false;
bool daemon_persist_control_state = true;
volatile sig_atomic_t daemon_running = 1;

static void on_signal(const int sig)
{
    (void)sig;

    daemon_running = 0;
}

int main(const int argc, char** argv)
{
    struct ans_config cfg;
    struct daemon_args args;

    hardware_names hardware_names;
    fan_state states[ANS_MAX_FANS] = {0};

    struct ec_device ec;

    daemon_runtime_state runtime = {0};

    bool auto_mode = true;
    bool coolboost_enabled = false;

    char preset[32] = "auto";
    const int args_result = daemon_args_parse(argc, argv, &args);

    if (args_result > 0)
        return 0;

    if (args_result < 0)
        return 2;

    if (config_load(args.config_path, &cfg) < 0 && (args.config_path_explicit || config_load(ANS_FALLBACK_CONFIG, &cfg) < 0))
    {
        fprintf(stderr, "failed to load config: %s\n", args.config_path);

        return 1;
    }

    coolboost_enabled = cfg.coolboost.default_enabled;
    runtime.power_source_auto_apply = cfg.power_source_profiles.auto_apply;

    keyboard_backlight_timeout_init(&cfg, &runtime);

    if (!args.force_model && !dmi_allowed(&cfg, load_dmi_model()))
    {
        fprintf(stderr, "refusing to run: DMI model is not allowed by config\n");

        return 1;
    }

    if (args.check_config)
    {
        printf("config ok: %s, %d fans, %d presets\n", cfg.model, cfg.fan_len, cfg.preset_len);

        return 0;
    }

    if (args.probe || args.validate)
    {
        if (ec_open_rw(cfg.ec_path, &ec) < 0)
        {
            print_ec_open_error();

            return 1;
        }

        if (args.validate)
        {
            validate_model(&ec, &cfg, args.config_path, args.force_model);
        }
        else
        {
            probe_ec(&ec, &cfg);
        }

        ec_close(&ec);

        return 0;
    }

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);
    signal(SIGPIPE, SIG_IGN);

    if (mkdir_p(ANS_RUN_DIR) < 0)
    {
        perror("mkdir " ANS_RUN_DIR);

        return 1;
    }

    if (mkdir_p(ANS_STATE_DIR) < 0)
    {
        perror("mkdir " ANS_STATE_DIR);

        return 1;
    }

    if (ec_open_rw(cfg.ec_path, &ec) < 0)
    {
        print_ec_open_error();

        return 1;
    }

    fprintf(stderr, "using EC backend: %s\n", ec.name);
    load_hardware_names(&hardware_names);

    const int sock_fd = make_socket();

    if (sock_fd < 0)
    {
        perror("control socket");
        ec_close(&ec);

        return 1;
    }

    for (int i = 0; i < cfg.fan_len; i++)
    {
        states[i].percent = cfg.fans[i].reset_speed;
        states[i].requested_percent = cfg.fans[i].reset_speed;
    }

    seed_last_temperatures(&cfg, states);

    apply_init_writes(&ec, &cfg);
    apply_sensor_power_control(&cfg, "on");

    if (restore_control_state(&ec, &cfg, states, &auto_mode, preset, sizeof(preset), &coolboost_enabled, &runtime))
    {
        /* persisted user choice wins over the model's default preset */
    }
    else if (strcmp(cfg.default_preset, "auto") == 0)
    {
        auto_mode = true;

        string_copy(preset, sizeof(preset), "auto");
        apply_daemon_control_fan_mode(&ec, &cfg);
        write_control_state(&cfg, states, auto_mode, preset, coolboost_enabled, &runtime);
    }
    else if (strcmp(cfg.default_preset, FIRMWARE_AUTO_PRESET) == 0 && apply_firmware_auto_fan_mode(&ec, &cfg))
    {
        auto_mode = false;
        string_copy(preset, sizeof(preset), FIRMWARE_AUTO_PRESET);
        write_control_state(&cfg, states, auto_mode, preset, coolboost_enabled,
                            &runtime);
    }
    else if (cfg.default_preset[0] && apply_preset(&ec, &cfg, states, cfg.default_preset))
    {
        auto_mode = false;

        string_copy(preset, sizeof(preset), cfg.default_preset);
        write_control_state(&cfg, states, auto_mode, preset, coolboost_enabled, &runtime);
    }

    run_daemon_loop(sock_fd, &ec, &cfg, states, &auto_mode, preset, sizeof(preset), &coolboost_enabled, &hardware_names, &runtime);

    apply_reset_writes(&ec, &cfg);
    apply_sensor_power_control(&cfg, "auto");

    close(sock_fd);
    ec_close(&ec);
    unlink(ANS_SOCKET_PATH);

    return 0;
}
