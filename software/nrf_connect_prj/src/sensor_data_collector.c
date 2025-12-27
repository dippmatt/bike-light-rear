#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>

#include "sensor_data_collector.h"
#include "utils.h"

#define SENSOR_THREAD_PRIORITY 7
#define SENSOR_THREAD_STACK_SIZE 1024
#define TIME_SAMPLING_INTERVAL_S 1

static const struct device *get_temp_sensor(void){

    const struct device *const dev = DEVICE_DT_GET(DT_NODELABEL(temp));

    if (dev == NULL) {
        /* No such node, or the node does not have status "okay"*/
        debug_printk("Error: Temperature sensor not found\n");
        return NULL;
    }

    if (!device_is_ready(dev)) {
        debug_printk("Error: Device %s not ready\n"
            "Check the driver initialization logs for errors.", dev->name);
        return NULL;
    }

    return dev;
}

static const struct device *get_light_sensor(void){

    const struct device *const dev = DEVICE_DT_GET(DT_NODELABEL(opt3001));

    if (dev == NULL) {
        /* No such node, or the node does not have status "okay"*/
        debug_printk("Error: OPT3001 device tree node not found\n");
        return NULL;
    }
    
    if (!device_is_ready(dev)) {
        debug_printk("Error: Device %s not ready\n"
            "Check the driver initialization logs for errors.\n", dev->name);
        return NULL;
    }

    return dev;
}

static const struct device *get_accel_sensor(void){

    const struct device *const dev = DEVICE_DT_GET(DT_NODELABEL(lis3dh));
    
    if (dev == NULL) {
        /* No such node, or the node does not have status "okay"*/
        debug_printk("Error: LIS3DH device tree node not found\n");
        return NULL;
    }
    
    if (!device_is_ready(dev)) {
        debug_printk("Error: Device %s not ready\n"
            "Check the driver initialization logs for errors.\n", dev->name);
        return NULL;
    }

    return dev;
}

void sensor_data_collector()
{

    const struct device *const temp_sensor = get_temp_sensor();
    const struct device *const light_sensor = get_light_sensor();
    const struct device *const accel_sensor = get_accel_sensor();

    if (temp_sensor == NULL && light_sensor == NULL && accel_sensor == NULL) {
        debug_printk("No sensors available\n");
        return;
    }

    while (1) {
        sensor_readings_t value;
        
        if (temp_sensor != NULL) {
            sensor_sample_fetch(temp_sensor);
            sensor_channel_get(temp_sensor, SENSOR_CHAN_DIE_TEMP, &value.temp);
            debug_printk("Temperature: %d.%06d\n", value.temp.val1, value.temp.val2);
        }
        
        if (light_sensor != NULL) {
            sensor_sample_fetch(light_sensor);
            sensor_channel_get(light_sensor, SENSOR_CHAN_LIGHT, &value.light);
            debug_printk("Light: %d.%06d lux\n", value.light.val1, value.light.val2);
        }
        else {
            debug_printk("No light sensor available\n");
        }
        
        if (accel_sensor != NULL) {
            sensor_sample_fetch(accel_sensor);
            sensor_channel_get(accel_sensor, SENSOR_CHAN_ACCEL_X, &value.accel_x);
            sensor_channel_get(accel_sensor, SENSOR_CHAN_ACCEL_Y, &value.accel_y);
            sensor_channel_get(accel_sensor, SENSOR_CHAN_ACCEL_Z, &value.accel_z);
            debug_printk("Acceleration X: %d.%06d, Y: %d.%06d, Z: %d.%06d m/s^2\n",
                   value.accel_x.val1, value.accel_x.val2,
                   value.accel_y.val1, value.accel_y.val2,
                   value.accel_z.val1, value.accel_z.val2);
        }
        else {
            debug_printk("No accelerometer available\n");
        }
        
        k_sleep(K_SECONDS(TIME_SAMPLING_INTERVAL_S));
    }
}

K_THREAD_DEFINE(sensor_data_collector_id, SENSOR_THREAD_STACK_SIZE, sensor_data_collector, NULL, 
        NULL, NULL, SENSOR_THREAD_PRIORITY, 0, 1000);