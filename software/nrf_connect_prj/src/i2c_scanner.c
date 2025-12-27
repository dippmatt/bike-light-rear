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

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>

#include "i2c_scanner.h"

static void read_lis3dh_who_am_i(const struct device *i2c_dev, uint8_t addr)
{
    uint8_t who_am_i_reg = 0x0F; /* WHO_AM_I register address */
    uint8_t who_am_i_value = 0;
    int ret;
    
    /* Read WHO_AM_I register (0x0F) from LIS3DH */
    ret = i2c_write_read(i2c_dev, addr, &who_am_i_reg, 1, &who_am_i_value, 1);
    if (ret == 0) {
        printk("  WHO_AM_I register at 0x%02X: 0x%02X (expected: 0x33 for LIS3DH)\n", 
               addr, who_am_i_value);
        if (who_am_i_value == 0x33) {
            printk("  ✓ LIS3DH detected correctly!\n");
        } else {
            printk("  ⚠ Unexpected WHO_AM_I value (might be LIS2DH or other variant)\n");
        }
    } else {
        printk("  Failed to read WHO_AM_I at 0x%02X (error: %d)\n", addr, ret);
    }
}

void scan_i2c_bus(const struct device *i2c_dev)
{
    uint8_t dummy = 0;
    int ret;
    int found = 0;

    printk("Scanning I2C bus...\n");
    
    /* Scan I2C addresses 0x08 to 0x77 (valid I2C address range) */
    /* Use write with dummy data to probe for devices */
    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
        /* Try to write 0 bytes - if device ACKs, it exists */
        ret = i2c_write(i2c_dev, &dummy, 0, addr);
        if (ret == 0) {
            printk("  Found device at address 0x%02X\n", addr);
            found++;
        }
        k_msleep(1); /* Small delay between probes */
    }
    
    if (found == 0) {
        printk("  No I2C devices found on bus\n");
    } else {
        printk("  Found %d device(s)\n", found);
    }
    
    /* Specifically check for known devices */
    printk("Checking specifically for known devices...\n");
    
    /* OPT3001 at 0x44 (ADDR to GND) */
    ret = i2c_write(i2c_dev, &dummy, 0, 0x44);
    if (ret == 0) {
        printk("  Device responds at 0x44 (OPT3001 with ADDR=GND)\n");
    } else {
        printk("  No response at 0x44 (error: %d)\n", ret);
    }
    
    /* LIS3DH at 0x18 (SA0 to GND) */
    ret = i2c_write(i2c_dev, &dummy, 0, 0x18);
    if (ret == 0) {
        printk("  Device responds at 0x18 (LIS3DH with SA0=GND)\n");
        read_lis3dh_who_am_i(i2c_dev, 0x18);
    } else {
        printk("  No response at 0x18 (error: %d)\n", ret);
    }
    
    /* LIS3DH at 0x19 (SA0 to VDD) */
    ret = i2c_write(i2c_dev, &dummy, 0, 0x19);
    if (ret == 0) {
        printk("  Device responds at 0x19 (LIS3DH with SA0=VDD)\n");
        read_lis3dh_who_am_i(i2c_dev, 0x19);
    } else {
        printk("  No response at 0x19 (error: %d)\n", ret);
    }
}

