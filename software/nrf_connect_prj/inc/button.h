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

#ifndef BUTTON_H
#define BUTTON_H

#include <zephyr/kernel.h>
#include <stdbool.h>

/**
 * @brief Button press callback function type
 * 
 * This callback is called when a button press is detected (after debounce).
 */
typedef void (*button_press_callback_t)(void);

/**
 * @brief Initialize button hardware and detection
 * 
 * Configures GPIO pin, sets up interrupts, and initializes debounce timer.
 * 
 * @param callback Function to call when button press is detected
 * @return 0 on success, negative error code on failure
 */
int button_init(button_press_callback_t callback);

#endif /* BUTTON_H */

