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

#ifndef BLE_H
#define BLE_H

/**
 * BLE peripheral: advertising and the light-control GATT service.
 *
 * Contract (the front light central depends on this):
 *   - Advertises 16-bit service UUID 0xA000
 *   - 0xA001: READ, one byte = current system state (0..4)
 *   - 0xA002: WRITE, one byte = requested system state (0..4)
 *
 * A valid write posts EVT_BLE_SET_STATE; the state machine applies its
 * stable-window guard and performs the transition.
 */

#define BLE_SERVICE_UUID    0xA000
#define BLE_LED_STATUS_UUID 0xA001
#define BLE_CONTROL_UUID    0xA002

/**
 * @brief Enable Bluetooth and start advertising
 *
 * Advertising failures are retried every second; a disconnect restarts
 * advertising.
 *
 * @return 0 on success, negative error code on failure
 */
int ble_init(void);

#endif /* BLE_H */
