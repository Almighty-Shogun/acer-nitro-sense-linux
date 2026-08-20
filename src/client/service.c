#include "client/service.h"

#include "core/constants.h"

#include <stdio.h>
#include <unistd.h>

int client_run_systemctl(const char *action)
{
    execlp("systemctl", "systemctl", action, "acer-nitro-sense.service", NULL);
    perror("systemctl");
    return 1;
}

int client_validate_model(void)
{
    execlp("acer-nitro-sensed", "acer-nitro-sensed", "--config", ANS_DEFAULT_CONFIG,
           "--validate-model", NULL);
    perror("acer-nitro-sensed");
    return 1;
}
