/*
 * Battery thresholds and ANT+ battery mapping, see battery_logic.h.
 */

#include "battery_logic.h"

#include "bike_light.h"
#include "config.h"

static bool low_battery;
static uint8_t consecutive_critical;
static uint8_t consecutive_low;
static uint8_t consecutive_high;

void battery_logic_reset(void)
{
    low_battery = false;
    consecutive_critical = 0U;
    consecutive_low = 0U;
    consecutive_high = 0U;
}

enum battery_level battery_logic_process(uint32_t mv)
{
    /* Critical: consecutive samples below the critical voltage */
    if (mv < BATTERY_CRITICAL_MV) {
        if (consecutive_critical < BATTERY_CONSECUTIVE) {
            consecutive_critical++;
        }
        if (consecutive_critical >= BATTERY_CONSECUTIVE) {
            return BATTERY_CRITICAL;
        }
    } else {
        consecutive_critical = 0U;
    }

    /* Low: consecutive samples below the low voltage; normal again after
     * as many at or above it */
    if (mv < BATTERY_LOW_MV) {
        if (consecutive_low < BATTERY_CONSECUTIVE) {
            consecutive_low++;
        }
        consecutive_high = 0U;
        if (consecutive_low >= BATTERY_CONSECUTIVE) {
            low_battery = true;
        }
    } else {
        if (consecutive_high < BATTERY_CONSECUTIVE) {
            consecutive_high++;
        }
        consecutive_low = 0U;
        if (consecutive_high >= BATTERY_CONSECUTIVE) {
            low_battery = false;
        }
    }

    return low_battery ? BATTERY_LOW : BATTERY_OK;
}

uint8_t battery_logic_ant_status(uint32_t mv, bool charging)
{
    if (charging) {
        return BL_BATT_CHARGING;
    }
    if (mv == 0U) {
        return BL_BATT_INVALID;
    }
    if (mv < BATTERY_CRITICAL_MV) {
        return BL_BATT_CRITICAL;
    }
    if (mv < BATTERY_LOW_MV) {
        return BL_BATT_LOW;
    }
    if (mv < BATTERY_STATUS_OK_MV) {
        return BL_BATT_OK;
    }
    if (mv < BATTERY_STATUS_GOOD_MV) {
        return BL_BATT_GOOD;
    }
    return BL_BATT_NEW;
}

uint8_t battery_logic_percent(uint32_t mv)
{
    if (mv == 0U) {
        return 0xFFU;
    }
    if (mv <= BATTERY_EMPTY_MV) {
        return 0U;
    }
    if (mv >= BATTERY_FULL_MV) {
        return 100U;
    }
    return (uint8_t)(((mv - BATTERY_EMPTY_MV) * 100U) / (BATTERY_FULL_MV - BATTERY_EMPTY_MV));
}
