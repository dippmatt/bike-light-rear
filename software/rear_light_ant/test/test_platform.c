/* Host stand-in for the platform lock: tests are single threaded */

#include "../src/platform.h"

uint8_t platform_lock(void)
{
    return 0U;
}

void platform_unlock(uint8_t key)
{
    (void)key;
}
