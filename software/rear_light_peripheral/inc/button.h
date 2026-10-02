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

#include <stdbool.h>

/**
 * User button input.
 *
 * Posts EVT_BUTTON_SHORT (released before 1 s) or EVT_BUTTON_LONG
 * (held for 1 s) to the central event queue. No callbacks, no state
 * machine knowledge.
 */

/**
 * @brief Configure the button GPIO as input (interrupts not yet enabled)
 *
 * @return 0 on success, negative error code on failure
 */
int button_init(void);

/**
 * @brief Enable button edge interrupts
 *
 * Call after slow initialization tasks (e.g. I2C scanning) to avoid
 * spurious button events during boot.
 *
 * @return 0 on success, negative error code on failure
 */
int button_enable(void);

/**
 * @brief Check whether the button is currently pressed
 *
 * @return true if pressed
 */
bool button_is_pressed(void);

/**
 * @brief Configure the button as System OFF wake source
 *
 * Stops press detection, disables the edge interrupt, removes the GPIO
 * callback and switches to a level-active interrupt so the nRF52 SENSE
 * mechanism can wake the chip from System OFF.
 *
 * @return 0 on success, negative error code on failure
 */
int button_prepare_wake(void);

/**
 * @brief Restore normal button operation after a failed power-off
 *
 * Undoes button_prepare_wake(): re-registers the GPIO callback and
 * re-enables the edge interrupt so the button keeps working if the
 * system could not enter System OFF.
 *
 * @return 0 on success, negative error code on failure
 */
int button_reenable(void);

#endif /* BUTTON_H */
