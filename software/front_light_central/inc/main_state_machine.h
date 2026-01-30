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

#ifndef MAIN_STATE_MACHINE_H
#define MAIN_STATE_MACHINE_H

#include "light_modes.h"

/**
 * @brief Initialize the main state machine
 * 
 * Sets up the state machine and connects button and LED control.
 * 
 * @return 0 on success, negative error code on failure
 */
int main_state_machine_init(void);

/**
 * @brief Handle button press event
 * 
 * Called by button module when a button press is detected.
 * This function cycles through LED brightness states.
 */
void main_state_machine_on_button_press(void);

/**
 * @brief Automatically turn off LED
 * 
 * Called by stationary monitor when device has been stationary for too long.
 * Only acts if current state is SMART_MODE.
 * 
 */
void main_state_machine_auto_off(void);

/**
 * @brief Get current LED state
 * 
 * @return Current LED brightness state
 */
enum system_state main_state_machine_get_state(void);

/**
 * @brief Get timestamp (ms) of last state transition
 */
int64_t main_state_machine_last_change_ms(void);

/**
 * @brief Peek next state without mutating the state machine
 *
 * @return Next state according to the normal cycle
 */

/**
 * @brief Set LED state directly (for external control, e.g., BLE)
 * 
 * Allows external modules to directly set the LED state.
 * This bypasses the normal button press cycle.
 * 
 * @param new_state The state to transition to
 * @return 0 on success, negative error code on failure
 */
int main_state_machine_set_state(enum system_state new_state);

/* Global LED state */
extern enum system_state g_system_state;

#endif /* MAIN_STATE_MACHINE_H */

