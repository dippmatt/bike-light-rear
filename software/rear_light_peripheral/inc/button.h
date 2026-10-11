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
 * @brief Arm the button as wake source for Deep Sleep
 *
 * Stops press detection and switches from the edge interrupt to a
 * one-shot level-active interrupt (nRF SENSE / PORT event, no GPIOTE
 * channel, lowest current). The first press posts EVT_BUTTON_WAKE and
 * disarms the interrupt again; if the button is already pressed the
 * event follows immediately.
 *
 * @return 0 on success, negative error code on failure
 */
int button_arm_wake(void);

/**
 * @brief Restore normal button operation
 *
 * Removes the wake interrupt (if armed), re-registers the press callback
 * and re-enables the edge interrupt. Used after waking from Deep Sleep
 * and to recover if the wake interrupt could not be armed. A button that
 * is still held produces no event; only a new press edge counts.
 *
 * @return 0 on success, negative error code on failure
 */
int button_reenable(void);

#endif /* BUTTON_H */
