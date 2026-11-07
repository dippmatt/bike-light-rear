/*
 * Battery thresholds and ANT+ battery mapping (SPEC section 8, ANT-12).
 * Pure logic: the voltage comes from the caller.
 */

#ifndef BATTERY_LOGIC_H
#define BATTERY_LOGIC_H

#include <stdbool.h>
#include <stdint.h>

enum battery_level {
    BATTERY_OK,
    BATTERY_LOW,
    BATTERY_CRITICAL,
};

/** Forget all history, as after a boot (also after every sleep) */
void battery_logic_reset(void);

/**
 * Process one sample. A failed reading must be passed as 0 mV (BAT-2).
 * Critical after BATTERY_CONSECUTIVE samples below BATTERY_CRITICAL_MV,
 * Low entered and left after BATTERY_CONSECUTIVE samples around BATTERY_LOW_MV.
 */
enum battery_level battery_logic_process(uint32_t mv);

/** ANT+ battery status for page 1 (enum bl_battery_status) */
uint8_t battery_logic_ant_status(uint32_t mv, bool charging);

/** Rough state of charge in percent for page 19, 0xFF if unknown */
uint8_t battery_logic_percent(uint32_t mv);

#endif /* BATTERY_LOGIC_H */
