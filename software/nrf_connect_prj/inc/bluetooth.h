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

/* Custom 128-bit UUID for Button Service */
#define BT_UUID_BUTTON_SERVICE_VAL \
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)

/**
 * @brief Initialize Bluetooth Low Energy stack
 * 
 * Initializes BLE in either master (central) or slave (peripheral) mode
 * based on BLE_MASTER preprocessor definition.
 * 
 * @return 0 on success, negative error code on failure
 */
int bluetooth_init(void);

#ifdef BLE_MASTER
/**
 * @brief Send button press to slave device (Master mode only)
 * 
 * Sends a button press command to the connected slave.
 * 
 * @return 0 on success, negative error code on failure
 */
int bluetooth_master_send_button_press(void);
#endif /* BLE_MASTER */

#endif /* BLUETOOTH_H */



