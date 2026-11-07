/*
 * Battery voltage by single-shot SAADC read (AIN5 behind a 1M/100k divider),
 * and the periodic tick (SPEC section 8).
 */

#ifndef BATTERY_H
#define BATTERY_H

#include <stdint.h>

#include "battery_logic.h"

/** @return battery voltage in mV, 0 if the conversion failed (BAT-2) */
uint32_t battery_read_mv(void);

/** Last voltage read by battery_sample() or battery_read_mv() */
uint32_t battery_last_mv(void);

/** Start the tick from a clean slate, like after a boot; first tick after one interval */
void battery_tick_start(void);
void battery_tick_stop(void);

/** Read the voltage and run the thresholds (BAT-1 .. BAT-4) */
enum battery_level battery_sample(void);

#endif /* BATTERY_H */
