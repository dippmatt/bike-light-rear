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

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Central state machine.
 *
 * Runs in the main thread (sm_run() never returns) and is the single
 * consumer of the central event queue. It is the only writer of the
 * system state and the only caller of light_set() / status_led_set().
 *
 * The numeric state values are part of the BLE contract (characteristic
 * 0xA001 read / 0xA002 write) and must not change.
 */
enum system_state {
    LED_OFF = 0,        /* light off, system awake (BLE reachable) */
    LED_50_PERCENT = 1, /* solid base brightness */
    LED_50_80_FLASH = 2, /* base brightness with periodic double flash */
    LED_SMART_MODE = 3, /* sensor-controlled brightness */
    IDLE_CHARGING = 4,  /* light off, USB connected */
};

/**
 * @brief Initialize the state machine and enter the boot state
 *
 * Runs the entry action of the boot state, so light_init() and
 * status_led_init() must have succeeded before this call.
 *
 * @param boot_state  State to boot into
 * @param usb_present USB VBUS state at boot
 */
void sm_init(enum system_state boot_state, bool usb_present);

/**
 * @brief Get the first Active Mode of the Mode Cycle
 *
 * The state entered by a wake or boot that turns the light on and by a
 * short press from Off or Idle Charging.
 *
 * @return First Active Mode
 */
enum system_state sm_first_active_mode(void);

/**
 * @brief Run the state machine event loop (never returns)
 */
void sm_run(void);

/**
 * @brief Get the current system state
 *
 * Safe to call from any context (atomic read); used by the BLE read
 * callback.
 *
 * @return Current state as uint8_t
 */
uint8_t sm_current_state(void);

#endif /* STATE_MACHINE_H */
