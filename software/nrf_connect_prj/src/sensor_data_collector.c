#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>

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

void sensor_data_collector()
{

    const struct device *const temp_sensor = get_temp_sensor();

    if (temp_sensor == NULL) {
        return;
    }

    while (1) {
        sensor_readings_t value;
        sensor_sample_fetch(temp_sensor);
        sensor_channel_get(temp_sensor, SENSOR_CHAN_DIE_TEMP, &value.temp);
        printk("Temperature: %d.%06d\n", value.temp.val1, value.temp.val2);
        k_sleep(K_SECONDS(TIME_SAMPLING_INTERVAL_S));
    }
}

K_THREAD_DEFINE(sensor_data_collector_id, SENSOR_THREAD_STACK_SIZE, sensor_data_collector, NULL, 
        NULL, NULL, SENSOR_THREAD_PRIORITY, 0, 1000);