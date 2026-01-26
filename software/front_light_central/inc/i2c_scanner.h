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

#ifndef I2C_SCANNER_H
#define I2C_SCANNER_H

#include <zephyr/device.h>

/**
 * @brief Scan I2C bus for devices
 * 
 * Scans the I2C bus and reports all found devices.
 * Also specifically checks for known devices (OPT3001, LIS3DH).
 * 
 * @param i2c_dev I2C device pointer (e.g., from DT_NODELABEL(i2c0))
 */
void scan_i2c_bus(const struct device *i2c_dev);

#endif /* I2C_SCANNER_H */

