#include "client/doctor_probes.h"

const char *DOCTOR_PROBE_DATE = "date --iso-8601=seconds 2>/dev/null || date";

const char *DOCTOR_PROBE_SERVICE_STATUS =
    "systemctl --no-pager --full status acer-nitro-sense.service 2>&1";

const char *DOCTOR_PROBE_CURRENT_DAEMON_JOURNAL =
    "since=$(systemctl show -P ActiveEnterTimestamp "
    "acer-nitro-sense.service 2>/dev/null); "
    "if [ -n \"$since\" ]; then "
    "journalctl -u acer-nitro-sense.service --since \"$since\" --no-pager 2>&1; "
    "else journalctl -u acer-nitro-sense.service -n 80 --no-pager 2>&1; fi";

const char *DOCTOR_PROBE_RECENT_DAEMON_JOURNAL =
    "journalctl -u acer-nitro-sense.service -n 80 --no-pager 2>&1";

const char *DOCTOR_PROBE_EC_ACCESS =
    "ls -l /dev/ec /sys/kernel/debug/ec/ec0/io 2>&1 || true";

const char *DOCTOR_PROBE_EC_MODULES =
    "lsmod | grep -E '(^acpi_ec|^ec_sys)' || true";

const char *DOCTOR_PROBE_HWMON =
    "for h in /sys/class/hwmon/hwmon*; do "
    "[ -e \"$h\" ] || continue; "
    "printf '%s name=' \"$h\"; "
    "cat \"$h/name\" 2>/dev/null || echo unknown; "
    "for t in \"$h\"/temp*_input; do "
    "[ -e \"$t\" ] || continue; "
    "printf '  %s=' \"$(basename \"$t\")\"; "
    "cat \"$t\" 2>&1; "
    "done; "
    "done";

const char *DOCTOR_PROBE_NVIDIA_PCI_POWER =
    "for d in /sys/bus/pci/devices/*; do "
    "[ -r \"$d/vendor\" ] || continue; "
    "if [ \"$(cat \"$d/vendor\")\" = \"0x10de\" ]; then "
    "echo \"$d\"; "
    "cat \"$d/device\" 2>/dev/null; "
    "cat \"$d/power/runtime_status\" 2>/dev/null; "
    "cat \"$d/power/control\" 2>/dev/null; "
    "fi; "
    "done";

const char *DOCTOR_PROBE_POWER_PROFILES_CTL =
    "command -v powerprofilesctl >/dev/null && "
    "powerprofilesctl 2>&1 || echo powerprofilesctl=unavailable";

const char *DOCTOR_PROBE_POWER_PROFILE_DAEMON =
    "systemctl --no-pager --full status power-profiles-daemon.service 2>&1 || true";

const char *DOCTOR_PROBE_PLATFORM_PROFILE =
    "ls -l /sys/firmware/acpi/platform_profile "
    "/sys/firmware/acpi/platform_profile_choices 2>&1; "
    "cat /sys/firmware/acpi/platform_profile "
    "/sys/firmware/acpi/platform_profile_choices 2>&1 || true";

const char *DOCTOR_PROBE_POWER_SUPPLY =
    "for p in /sys/class/power_supply/*; do "
    "[ -e \"$p\" ] || continue; "
    "echo \"$p\"; "
    "for f in type online status capacity charge_now charge_full energy_now "
    "energy_full power_now voltage_now manufacturer model_name; do "
    "[ -r \"$p/$f\" ] && printf '  %s=' \"$f\" && cat \"$p/$f\" 2>&1; "
    "done; "
    "done";

const char *DOCTOR_PROBE_LED_LIST = "ls -l /sys/class/leds 2>&1";

const char *DOCTOR_PROBE_LED_DETAILS =
    "for l in /sys/class/leds/*; do "
    "[ -e \"$l\" ] || continue; "
    "echo \"$l\"; "
    "for f in brightness max_brightness trigger delay_on delay_off; do "
    "[ -r \"$l/$f\" ] && printf '  %s=' \"$f\" && cat \"$l/$f\" 2>&1; "
    "done; "
    "[ -e \"$l/device\" ] && printf '  device=' && readlink -f \"$l/device\" 2>&1; "
    "done; true";
