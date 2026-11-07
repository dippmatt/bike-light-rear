/*
 * Platform services for the portable core, see platform.h, plus the few
 * helpers that every module needs (delay, reset).
 */

#include "platform.h"

#include "sd_includes.h"
#include "soft_timer.h"

uint8_t platform_lock(void)
{
    uint8_t nested = 0U;

    (void)sd_nvic_critical_region_enter(&nested);
    return nested;
}

void platform_unlock(uint8_t key)
{
    (void)sd_nvic_critical_region_exit(key);
}
