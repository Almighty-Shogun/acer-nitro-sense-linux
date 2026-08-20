#include "client/doctor.h"

#include "client/doctor_util.h"
#include "client/transport.h"

#include <stdio.h>

int client_doctor(void)
{
    char status_response[4096];

    doctor_print_section("Acer Nitro Sense Doctor");
    doctor_run_command("date", "date --iso-8601=seconds 2>/dev/null || date");
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
    doctor_run_command("which", "command -v acer-nitro-sense; command -v ans; command -v acer-nitro-sensed");
    doctor_run_command("config", "ls -l /etc/acer-nitro-sense/model.json 2>&1; readlink -f /etc/acer-nitro-sense/model.json 2>&1");
    doctor_print_socket_permissions();
    doctor_print_user_groups();

    doctor_print_section("Service");
    doctor_run_command("systemctl", "systemctl --no-pager --full status acer-nitro-sense.service 2>&1");
    doctor_run_command("journal_current_daemon", "since=$(systemctl show -P ActiveEnterTimestamp acer-nitro-sense.service 2>/dev/null); if [ -n \"$since\" ]; then journalctl -u acer-nitro-sense.service --since \"$since\" --no-pager 2>&1; else journalctl -u acer-nitro-sense.service -n 80 --no-pager 2>&1; fi");
    doctor_run_command("journal", "journalctl -u acer-nitro-sense.service -n 80 --no-pager 2>&1");

    doctor_print_section("EC Access");
    doctor_run_command("ec", "ls -l /dev/ec /sys/kernel/debug/ec/ec0/io 2>&1 || true");
    doctor_run_command("modules", "lsmod | grep -E '(^acpi_ec|^ec_sys)' || true");
    doctor_print_file_value("kernel_lockdown", "/sys/kernel/security/lockdown");

    doctor_print_section("Sensors");
    doctor_run_command("hwmon", "for h in /sys/class/hwmon/hwmon*; do [ -e \"$h\" ] || continue; printf '%s name=' \"$h\"; cat \"$h/name\" 2>/dev/null || echo unknown; for t in \"$h\"/temp*_input; do [ -e \"$t\" ] || continue; printf '  %s=' \"$(basename \"$t\")\"; cat \"$t\" 2>&1; done; done");
    doctor_run_command("nvidia", "for d in /sys/bus/pci/devices/*; do [ -r \"$d/vendor\" ] || continue; if [ \"$(cat \"$d/vendor\")\" = \"0x10de\" ]; then echo \"$d\"; cat \"$d/device\" 2>/dev/null; cat \"$d/power/runtime_status\" 2>/dev/null; cat \"$d/power/control\" 2>/dev/null; fi; done");

    doctor_print_section("Power Profiles");
    doctor_run_command("powerprofilesctl", "command -v powerprofilesctl >/dev/null && powerprofilesctl 2>&1 || echo powerprofilesctl=unavailable");
    doctor_run_command("powerprofilesd", "systemctl --no-pager --full status power-profiles-daemon.service 2>&1 || true");
    doctor_run_command("platform_profile", "ls -l /sys/firmware/acpi/platform_profile /sys/firmware/acpi/platform_profile_choices 2>&1; cat /sys/firmware/acpi/platform_profile /sys/firmware/acpi/platform_profile_choices 2>&1 || true");

    doctor_print_section("AC And Battery");
    doctor_run_command("power_supply", "for p in /sys/class/power_supply/*; do [ -e \"$p\" ] || continue; echo \"$p\"; for f in type online status capacity charge_now charge_full energy_now energy_full power_now voltage_now manufacturer model_name; do [ -r \"$p/$f\" ] && printf '  %s=' \"$f\" && cat \"$p/$f\" 2>&1; done; done");

    doctor_print_section("Keyboard Backlight");
    doctor_run_command("keyboard_backlight", "acer-nitro-sense keyboard-backlight status 2>&1");
    doctor_run_command("leds", "ls -l /sys/class/leds 2>&1");
    doctor_run_command("led_details", "for l in /sys/class/leds/*; do [ -e \"$l\" ] || continue; echo \"$l\"; for f in brightness max_brightness trigger delay_on delay_off; do [ -r \"$l/$f\" ] && printf '  %s=' \"$f\" && cat \"$l/$f\" 2>&1; done; [ -e \"$l/device\" ] && printf '  device=' && readlink -f \"$l/device\" 2>&1; done; true");

    return 0;
}
