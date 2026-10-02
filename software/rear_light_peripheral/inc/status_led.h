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

#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <stdbool.h>

/**
 * Status LED (single GPIO indicator).
 *
 * Display priority (highest first):
 *   1. low battery  -> 500 ms blink
 *   2. charging     -> 300 ms blink
 *   3. light on     -> solid on
 *   4. otherwise    -> off
 *
 * OWNERSHIP: only the state machine thread may call status_led_set() /
 * status_led_off(). The module owns its blink timer internally.
 */

/**
 * @brief Initialize the status LED GPIO (configured off)
 *
 * @return 0 on success, negative error code on failure
 */
int status_led_init(void);

/**
 * @brief Update the status LED according to the display priority
 *
 * @param low_battery Battery below the low threshold
 * @param charging    USB VBUS present
 * @param light_on    Main light in an active mode (50%, FLASH or SMART)
 */
void status_led_set(bool low_battery, bool charging, bool light_on);

/**
 * @brief Turn the status LED off and stop blinking (shutdown path)
 */
void status_led_off(void);

#endif /* STATUS_LED_H */
