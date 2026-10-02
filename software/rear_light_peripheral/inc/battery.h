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

#ifndef BATTERY_H
#define BATTERY_H

#include <stdint.h>

/**
 * Battery voltage monitoring via ADC (1M/100k divider on AIN5).
 *
 * A 5 s timer posts EVT_BATTERY_TICK; the state machine thread then calls
 * battery_sample_and_process(). All hysteresis state lives in this module
 * and is only touched from the state machine context.
 */

enum battery_level {
    BATTERY_OK,       /* normal operation */
    BATTERY_LOW,      /* below 3.4 V (3 consecutive samples, with hysteresis) */
    BATTERY_CRITICAL, /* below 3.0 V for 3 consecutive samples - power off */
};

/**
 * @brief Set up the battery ADC channel and start the 5 s sampling tick
 *
 * A failed ADC setup is retried on each sample.
 *
 * @return 0 on success, negative error code if the ADC is unavailable
 */
int battery_init(void);

/**
 * @brief Sample the battery voltage and update the level hysteresis
 *
 * Must only be called from the state machine thread (on EVT_BATTERY_TICK).
 *
 * @return Current battery level after hysteresis
 */
enum battery_level battery_sample_and_process(void);

/**
 * @brief Get the most recent battery voltage reading
 *
 * @return Battery voltage in mV (0 if no successful reading yet)
 */
uint16_t battery_last_mv(void);

#endif /* BATTERY_H */
