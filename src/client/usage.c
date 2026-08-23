#include "client/usage.h"

/**
 * Print command-line usage.
 *
 * The CLI uses this helper to keep terminal output and daemon transport
 * behavior consistent across subcommands.
 */
void client_usage(FILE* out)
{
    fprintf(out,
            "usage:\n"
            "  acer-nitro-sense start|stop|restart|status|capabilities|auto|firmware-auto|presets|resume|validate|doctor\n"
            "  acer-nitro-sense status --json|--celsius|--fahrenheit\n"
            "  acer-nitro-sense coolboost on|off|status\n"
            "  acer-nitro-sense fan-mode status|auto|manual|turbo\n"
            "  acer-nitro-sense profile status|quiet|balanced|performance\n"
            "  acer-nitro-sense gpu-temp status|auto|live\n"
            "  acer-nitro-sense power-source status|apply|auto on|auto off\n"
            "  acer-nitro-sense keyboard-backlight status|set 0-100|timeout status|on|off\n"
            "  acer-nitro-sense ec read REG\n"
            "  acer-nitro-sense ec dump START END\n"
            "  acer-nitro-sense set cpu|gpu|all 1-100\n"
            "  acer-nitro-sense preset show|silent|quiet|balanced|cool|performance|turbo|max\n");
}
