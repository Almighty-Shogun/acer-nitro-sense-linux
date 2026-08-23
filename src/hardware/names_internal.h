#ifndef ANS_HARDWARE_NAMES_INTERNAL_H
#define ANS_HARDWARE_NAMES_INTERNAL_H

#include <stddef.h>

/**
 * Load the CPU model name.
 *
 * The name is read from Linux CPU metadata and bounded to the caller's output
 * buffer.
 */
void load_cpu_name(char* out, size_t out_len);

/**
 * Load the discrete GPU model name.
 *
 * The name is read from PCI metadata when a supported GPU is present.
 */
void load_gpu_name(char* out, size_t out_len);

#endif
