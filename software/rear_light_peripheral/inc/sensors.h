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

#ifndef SENSORS_H
#define SENSORS_H

/**
 * Environmental sensing for SMART mode (LIS3DH accelerometer + OPT3001
 * ambient light sensor).
 *
 * A dedicated thread samples both sensors on their own schedule while
 * started (accelerometer and light sensor intervals are independent of
 * the detection times, which are specified in milliseconds). All
 * detection hysteresis lives inside this module; only edge events are
 * posted to the central event queue:
 *   EVT_BRAKE_START / EVT_BRAKE_STOP
 *   EVT_AMBIENT_DARK / EVT_AMBIENT_BRIGHT
 *   EVT_STATIONARY_TIMEOUT
 *
 * The thread never touches the light output.
 */

/**
 * @brief Check sensor availability (devices are resolved at compile time)
 *
 * @return 0 if at least one sensor is ready, -ENODEV if none
 */
int sensors_init(void);

/**
 * @brief Start sensor sampling (entering SMART mode)
 *
 * Resets all detectors before sampling begins, so a re-entered SMART
 * mode never acts on stale detector state.
 */
void sensors_start(void);

/**
 * @brief Stop sensor sampling (leaving SMART mode)
 */
void sensors_stop(void);

#endif /* SENSORS_H */
