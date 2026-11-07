/*
 * The few things the portable core needs from the platform. The target
 * implements them in platform.c, the host tests in test/test_platform.c.
 */

#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>

/** Enter a critical section that excludes the interrupts posting events */
uint8_t platform_lock(void);
void platform_unlock(uint8_t key);

#endif /* PLATFORM_H */
