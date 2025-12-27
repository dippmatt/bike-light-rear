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

