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

#ifndef LIGHT_H
#define LIGHT_H

#include <stdint.h>

/**
 * Main LED PWM driver (PAM2804 dimming input).
 *
 * OWNERSHIP: only the state machine thread (state_machine.c) may call
 * light_set(), so it alone decides what the light shows.
 *
 * The one exception is the blank: the sensor thread may switch the output
 * off for a short time to measure ambient light without the LED's own
 * light (light_blank_begin() / light_blank_end()). While blanked,
 * light_set() only records the requested level; light_blank_end() applies
 * the latest request. The gate is serialized by a mutex.
 */

/* PWM period: 1 kHz */
#define LIGHT_PWM_PERIOD_USEC 1000U

/* Pulse widths (microseconds within the 1000 us period) */
#define LIGHT_PULSE_OFF  0U
#define LIGHT_PULSE_BASE 200U /* normal riding brightness */
#define LIGHT_PULSE_PEAK 800U /* brake / flash peak brightness */

/**
 * @brief Initialize the main LED PWM (period set, output off)
 *
 * @return 0 on success, negative error code on failure
 */
int light_init(void);

/**
 * @brief Set the main LED pulse width
 *
 * @param pulse_usec Pulse width in microseconds (0 = off)
 */
void light_set(uint32_t pulse_usec);

/**
 * @brief Switch the main LED off for an ambient light measurement
 *
 * Only for the sensor thread. The level requested through light_set()
 * is kept and restored by light_blank_end(). Every call must be paired
 * with light_blank_end(), also on errors.
 */
void light_blank_begin(void);

/**
 * @brief End a blank and apply the latest level requested via light_set()
 */
void light_blank_end(void);

#endif /* LIGHT_H */
