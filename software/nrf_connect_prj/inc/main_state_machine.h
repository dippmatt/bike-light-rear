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

/* State function pointer types */
typedef void (*state_init_fn)(void);
typedef void (*state_terminate_fn)(void);

/* State descriptor */
typedef struct {
    enum led_brightness state_id;
    state_init_fn init;
    state_terminate_fn terminate;
    enum led_brightness next_state;
} state_descriptor_t;

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
 * @param enter_sleep If true, system will enter sleep mode after turning off
 */
void main_state_machine_auto_off(bool enter_sleep);

/**
 * @brief Get current LED state
 * 
 * @return Current LED brightness state
 */
enum led_brightness main_state_machine_get_state(void);

/* Global LED state */
extern enum led_brightness g_led_state;

#endif /* MAIN_STATE_MACHINE_H */

