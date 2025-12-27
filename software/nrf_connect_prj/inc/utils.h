#ifndef UTILS_H
#define UTILS_H

#include <zephyr/kernel.h>

/**
 * @brief Debug print function that conditionally uses printk
 * 
 * When DEBUG is enabled, this function behaves like printk().
 * When DEBUG is disabled, this function does nothing (no code generated).
 * 
 * Usage: debug_printk("format string", args...)
 */
#ifdef DEBUG
#define debug_printk(...) printk(__VA_ARGS__)
#else
#define debug_printk(...) ((void)0)
#endif

#endif /* UTILS_H */

