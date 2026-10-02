/*
 * Copyright (c) 2025 Matthias Dippold
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * NON-COMMERCIAL USE CLAUSE:
 * Permission is granted to use, copy, modify, and distribute this software
 * and its derivatives for NON-COMMERCIAL purposes only. Commercial use of
 * this software, in whole or in part, is strictly prohibited without
 * explicit written permission from the copyright holder.
 */

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>

#include "battery.h"
#include "app_events.h"
#include "debug.h"

/* Battery voltage divider: 1M to VBATT, 100k to GND -> V_AIN5 = VBATT/11 */
#define BATTERY_DIVIDER_RATIO 11U
/* nRF SAADC internal ref 0.6V, 12-bit; (raw * 600 * 11) / 4096 = VBATT_mV */
#define BATTERY_RAW_TO_MV(raw) ((uint32_t)(raw) * 600U * BATTERY_DIVIDER_RATIO / 4096U)

#define BATTERY_CRITICAL_MV        3000
#define BATTERY_LOW_MV             3400
#define BATTERY_CONSECUTIVE        3
#define BATTERY_SAMPLE_INTERVAL_MS 5000

static const struct adc_dt_spec battery_adc_spec = ADC_DT_SPEC_GET(DT_ALIAS(battery_adc));
static bool adc_setup_done;

static struct k_timer sample_tick_timer;

/* Hysteresis state; only touched from the state machine context */
static uint16_t last_mv;
static bool low_battery;
static uint8_t consecutive_critical;
static uint8_t consecutive_low;
static uint8_t consecutive_high;

static void sample_tick_expiry(struct k_timer *timer)
{
    ARG_UNUSED(timer);
    app_event_post(EVT_BATTERY_TICK, 0);
}

int battery_init(void)
{
    int ret = adc_channel_setup_dt(&battery_adc_spec);

    if (ret == 0) {
        adc_setup_done = true;
    } else {
        debug_printk("Battery ADC setup failed (%d), will retry on sampling\n", ret);
    }

    k_timer_init(&sample_tick_timer, sample_tick_expiry, NULL);
    k_timer_start(&sample_tick_timer, K_MSEC(BATTERY_SAMPLE_INTERVAL_MS),
                  K_MSEC(BATTERY_SAMPLE_INTERVAL_MS));

    return ret;
}

enum battery_level battery_sample_and_process(void)
{
    uint16_t mv = 0;

    if (!adc_setup_done) {
        if (adc_channel_setup_dt(&battery_adc_spec) == 0) {
            adc_setup_done = true;
        }
    }

    if (adc_setup_done && adc_is_ready_dt(&battery_adc_spec)) {
        int32_t raw = 0;
        struct adc_sequence seq = {
            .buffer = &raw,
            .buffer_size = sizeof(raw),
            .channels = BIT(battery_adc_spec.channel_id),
            .resolution = battery_adc_spec.resolution,
        };

        if (adc_read_dt(&battery_adc_spec, &seq) == 0) {
            mv = (uint16_t)BATTERY_RAW_TO_MV((uint32_t)raw);
            last_mv = mv;
            debug_printk("Battery: %u mV\n", mv);
        }
    }

    /* Critical: 3 consecutive samples below 3000 mV. A failed reading counts
     * as 0 mV on purpose - a battery we cannot measure is treated as empty. */
    if (mv < BATTERY_CRITICAL_MV) {
        if (consecutive_critical < BATTERY_CONSECUTIVE) {
            consecutive_critical++;
        }
        if (consecutive_critical >= BATTERY_CONSECUTIVE) {
            return BATTERY_CRITICAL;
        }
    } else {
        consecutive_critical = 0;
    }

    /* Low battery: 3 consecutive < 3400 mV -> low; 3 consecutive >= 3400 -> normal */
    if (mv < BATTERY_LOW_MV) {
        consecutive_low++;
        consecutive_high = 0;
        if (consecutive_low >= BATTERY_CONSECUTIVE) {
            low_battery = true;
        }
    } else {
        consecutive_high++;
        consecutive_low = 0;
        if (consecutive_high >= BATTERY_CONSECUTIVE) {
            low_battery = false;
        }
    }

    return low_battery ? BATTERY_LOW : BATTERY_OK;
}

uint16_t battery_last_mv(void)
{
    return last_mv;
}
