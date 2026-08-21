#include "client/doctor.h"

#include "client/doctor_probes.h"
#include "client/doctor_util.h"
#include "client/transport.h"

#include <stdio.h>

int client_doctor(void)
{
    char status_response[4096];

    doctor_print_section("Acer Nitro Sense Doctor");
    doctor_run_command("date", DOCTOR_PROBE_DATE);
    doctor_run_command("uname", "uname -a");
    doctor_print_os_pretty_name();

    doctor_print_section("DMI");
    doctor_print_file_value("product_name", "/sys/class/dmi/id/product_name");
    doctor_print_file_value("product_version", "/sys/class/dmi/id/product_version");
    doctor_print_file_value("board_name", "/sys/class/dmi/id/board_name");
    doctor_print_file_value("bios_version", "/sys/class/dmi/id/bios_version");

    doctor_print_section("ANS Status");

    if (client_send_command_capture("status\n", true, status_response, sizeof(status_response)) == 0)
        fputs(status_response, stdout);
    else
        client_print_status_file();

    doctor_run_command("coolboost", "acer-nitro-sense coolboost status 2>&1");
    doctor_run_command("fan_mode", "acer-nitro-sense fan-mode status 2>&1");
    doctor_run_command("profile", "acer-nitro-sense profile status 2>&1");
    doctor_run_command("gpu_temp", "acer-nitro-sense gpu-temp status 2>&1");

    doctor_print_section("Install And Permissions");
    doctor_run_command("which", DOCTOR_PROBE_INSTALL_PATHS);
    doctor_run_command("config", DOCTOR_PROBE_MODEL_CONFIG);
    doctor_print_socket_permissions();
    doctor_print_user_groups();

    doctor_print_section("Service");
    doctor_run_command("systemctl", DOCTOR_PROBE_SERVICE_STATUS);
    doctor_run_command("journal_current_daemon",
                       DOCTOR_PROBE_CURRENT_DAEMON_JOURNAL);
    doctor_run_command("journal", DOCTOR_PROBE_RECENT_DAEMON_JOURNAL);

    doctor_print_section("EC Access");
    doctor_run_command("ec", DOCTOR_PROBE_EC_ACCESS);
    doctor_run_command("modules", DOCTOR_PROBE_EC_MODULES);
    doctor_print_file_value("kernel_lockdown", "/sys/kernel/security/lockdown");

    doctor_print_section("Sensors");
    doctor_run_command("hwmon", DOCTOR_PROBE_HWMON);
    doctor_run_command("nvidia", DOCTOR_PROBE_NVIDIA_PCI_POWER);

    doctor_print_section("Power Profiles");
    doctor_run_command("powerprofilesctl", DOCTOR_PROBE_POWERPROFILESCTL);
    doctor_run_command("powerprofilesd", DOCTOR_PROBE_POWER_PROFILE_DAEMON);
    doctor_run_command("platform_profile", DOCTOR_PROBE_PLATFORM_PROFILE);

    doctor_print_section("AC And Battery");
    doctor_run_command("power_supply", DOCTOR_PROBE_POWER_SUPPLY);

    doctor_print_section("Keyboard Backlight");
    doctor_run_command("keyboard_backlight", "acer-nitro-sense keyboard-backlight status 2>&1");
    doctor_run_command("leds", DOCTOR_PROBE_LED_LIST);
    doctor_run_command("led_details", DOCTOR_PROBE_LED_DETAILS);

    return 0;
}
