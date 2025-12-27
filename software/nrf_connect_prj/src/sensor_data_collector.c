#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/i2c.h>

#include "sensor_data_collector.h"

#define SENSOR_THREAD_PRIORITY 7
#define SENSOR_THREAD_STACK_SIZE 1024
#define TIME_SAMPLING_INTERVAL_S 1

static const struct device *get_temp_sensor(void){
    const struct device *const dev = DEVICE_DT_GET(DT_NODELABEL(temp));

    if (dev == NULL) {
        /* No such node, or the node does not have status "okay"*/
        printk("Error: Temperature sensor not found\n");
        return NULL;
    }

    if (!device_is_ready(dev)) {
        printk("Error: Device %s not ready\n"
            "Check the driver initialization logs for errors.", dev->name);
        return NULL;
    }
    printk("Found temperature sensor: %s\n", dev->name);
    return dev;
}

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

static void scan_i2c_bus(const struct device *i2c_dev)
{
    uint8_t dummy = 0;
    int ret;
    int found = 0;

    printk("Scanning I2C bus...\n");
    
    /* Scan I2C addresses 0x08 to 0x77 (valid I2C address range) */
    /* Use write with dummy data to probe for devices */
    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
        /* Try to write 1 byte - if device ACKs, it exists */
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

static const struct device *get_light_sensor(void){
    const struct device *const dev = DEVICE_DT_GET(DT_NODELABEL(opt3001));
    const struct device *const i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));

    printk("Checking OPT3001 device tree node...\n");
    
    if (dev == NULL) {
        /* No such node, or the node does not have status "okay"*/
        printk("Error: OPT3001 device tree node not found\n");
        
        /* Check if I2C bus is available and scan it */
        if (i2c_dev != NULL && device_is_ready(i2c_dev)) {
            printk("I2C bus is ready, scanning for devices...\n");
            scan_i2c_bus(i2c_dev);
        } else {
            printk("I2C bus not ready or not found\n");
        }
        return NULL;
    }

    printk("OPT3001 device tree node found: %s\n", dev->name);
    
    if (!device_is_ready(dev)) {
        printk("Error: Device %s not ready\n"
            "Check the driver initialization logs for errors.\n", dev->name);
        
        /* Try to scan I2C bus anyway */
        if (i2c_dev != NULL && device_is_ready(i2c_dev)) {
            printk("Scanning I2C bus for devices...\n");
            scan_i2c_bus(i2c_dev);
        }
        return NULL;
    }
    printk("Found OPT3001 light sensor: %s\n", dev->name);
    return dev;
}

static const struct device *get_accel_sensor(void){
    const struct device *const dev = DEVICE_DT_GET(DT_NODELABEL(lis3dh));
    const struct device *const i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));

    printk("Checking LIS3DH device tree node...\n");
    
    if (dev == NULL) {
        /* No such node, or the node does not have status "okay"*/
        printk("Error: LIS3DH device tree node not found\n");
        
        /* Check if I2C bus is available and scan it */
        if (i2c_dev != NULL && device_is_ready(i2c_dev)) {
            printk("I2C bus is ready, scanning for devices...\n");
            scan_i2c_bus(i2c_dev);
        } else {
            printk("I2C bus not ready or not found\n");
        }
        return NULL;
    }

    printk("LIS3DH device tree node found: %s\n", dev->name);
    
    if (!device_is_ready(dev)) {
        printk("Error: Device %s not ready\n"
            "Check the driver initialization logs for errors.\n", dev->name);
        
        /* Try to scan I2C bus anyway */
        if (i2c_dev != NULL && device_is_ready(i2c_dev)) {
            printk("Scanning I2C bus for devices...\n");
            scan_i2c_bus(i2c_dev);
        }
        return NULL;
    }
    printk("Found LIS3DH accelerometer: %s\n", dev->name);
    return dev;
}

void sensor_data_collector()
{

    const struct device *const temp_sensor = get_temp_sensor();
    const struct device *const light_sensor = get_light_sensor();
    const struct device *const accel_sensor = get_accel_sensor();

    if (temp_sensor == NULL && light_sensor == NULL && accel_sensor == NULL) {
        printk("No sensors available\n");
        return;
    }

    while (1) {
        sensor_readings_t value;
        
        if (temp_sensor != NULL) {
            sensor_sample_fetch(temp_sensor);
            sensor_channel_get(temp_sensor, SENSOR_CHAN_DIE_TEMP, &value.temp);
            printk("Temperature: %d.%06d\n", value.temp.val1, value.temp.val2);
        }
        
        if (light_sensor != NULL) {
            sensor_sample_fetch(light_sensor);
            sensor_channel_get(light_sensor, SENSOR_CHAN_LIGHT, &value.light);
            printk("Light: %d.%06d lux\n", value.light.val1, value.light.val2);
        }
        else {
            printk("No light sensor available\n");
        }
        
        if (accel_sensor != NULL) {
            sensor_sample_fetch(accel_sensor);
            sensor_channel_get(accel_sensor, SENSOR_CHAN_ACCEL_X, &value.accel_x);
            sensor_channel_get(accel_sensor, SENSOR_CHAN_ACCEL_Y, &value.accel_y);
            sensor_channel_get(accel_sensor, SENSOR_CHAN_ACCEL_Z, &value.accel_z);
            printk("Acceleration X: %d.%06d, Y: %d.%06d, Z: %d.%06d m/s^2\n",
                   value.accel_x.val1, value.accel_x.val2,
                   value.accel_y.val1, value.accel_y.val2,
                   value.accel_z.val1, value.accel_z.val2);
        }
        else {
            printk("No accelerometer available\n");
        }
        
        k_sleep(K_SECONDS(TIME_SAMPLING_INTERVAL_S));
    }
}

K_THREAD_DEFINE(sensor_data_collector_id, SENSOR_THREAD_STACK_SIZE, sensor_data_collector, NULL, 
        NULL, NULL, SENSOR_THREAD_PRIORITY, 0, 1000);