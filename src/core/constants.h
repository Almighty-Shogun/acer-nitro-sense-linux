#ifndef ANS_CORE_CONSTANTS_H
#define ANS_CORE_CONSTANTS_H

#define ANS_MAX_FANS 2
#define ANS_MAX_THRESHOLDS 32
#define ANS_MAX_WRITES 8
#define ANS_MAX_PRESETS 16
#define ANS_MAX_PLATFORM_PROFILES 8

#define ANS_RUN_DIR "/run/acer-nitro-sense"
#define ANS_SOCKET_PATH ANS_RUN_DIR "/control.sock"
#define ANS_STATUS_PATH ANS_RUN_DIR "/status.json"
#define ANS_TEMP_CACHE_PATH ANS_RUN_DIR "/temperature-cache.json"
#define ANS_STATE_DIR "/var/lib/acer-nitro-sense"
#define ANS_STATE_PATH ANS_STATE_DIR "/state.json"
#define ANS_DEFAULT_CONFIG "/etc/acer-nitro-sense/model.json"
#define ANS_MODEL_DIR "/usr/share/acer-nitro-sense/models"
#define ANS_FALLBACK_CONFIG "models/acer-nitro-an517-51.json"
#define ANS_CONTROL_GROUP "acer-nitro-sense"

#endif
