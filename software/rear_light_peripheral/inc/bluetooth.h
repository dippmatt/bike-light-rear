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

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#ifdef BLUETOOTH


#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <stdint.h>
#include <sys/types.h>

/**
 * @brief Initialize Bluetooth Low Energy stack
 * 
 * Initializes BLE in either master (central) or slave (peripheral) mode
 * based on BLE_MASTER preprocessor definition.
 * 
 * @return 0 on success, negative error code on failure
 */
int bluetooth_init(void);

#ifdef BLE_MASTER /* MASTER MODE */
/**
 * @brief Send button press command to remote BLE device (Master mode only)
 * 
 * Scans for, connects to, and sends a button press command to a remote BLE device.
 * The function writes a trigger value to the peripheral, which causes it to advance
 * its state machine (simulating a button press on the peripheral device).
 * The peripheral reads its own current state and advances to the next state in the cycle.
 * If the operation is already in progress (semaphore taken), this function
 * returns immediately without queuing a new operation.
 * 
 * @return void (no return value)
 */
void ble_toogle_led();
#endif /* END BLE_MASTER */

#endif /* BLUETOOTH */

#endif /* BLUETOOTH_H */



