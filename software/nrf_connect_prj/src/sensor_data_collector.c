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
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <math.h>

#include "sensor_data_collector.h"
#include "utils.h"
#include "light_modes.h"
#include "main_state_machine.h"
#include "power_management.h"

#define SENSOR_THREAD_PRIORITY 7
#define SENSOR_THREAD_STACK_SIZE 1024
#define STATIONARY_THREAD_STACK_SIZE 1024
#define STATIONARY_THREAD_PRIORITY 8
#define TIME_SAMPLING_INTERVAL_MS 500

/* Braking detection thresholds */
#define BRAKING_ACCEL_THRESHOLD -3.0
#define AMBIENT_DARK_THRESHOLD 50.0
#define AMBIENT_BRIGHT_THRESHOLD 150.0
#define AMBIENT_DARK_SAMPLES_REQUIRED 3

/* Thread control */
static k_tid_t sensor_thread_id = NULL;
static k_tid_t stationary_thread_id = NULL;
static bool threads_running = false;

K_THREAD_STACK_DEFINE(sensor_stack, SENSOR_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(stationary_stack, STATIONARY_THREAD_STACK_SIZE);
static struct k_thread sensor_thread_data;
static struct k_thread stationary_thread_data;

/* State variables */
static bool prev_sample_braking = false;
static uint8_t ambient_dark_sample_count = 0;
static uint8_t ambient_bright_sample_count = 0;

static void write_sensor_data(const sensor_readings_t *value);
static void update_environmental_state(const sensor_readings_t *value);

static const struct device *get_temp_sensor(void)
{
    const struct device *const dev = DEVICE_DT_GET(DT_NODELABEL(temp));
    if (dev == NULL || !device_is_ready(dev)) {
        debug_printk("Error: Temperature sensor not ready\n");
        return NULL;
    }
    return dev;
}

static const struct device *get_light_sensor(void)
{
    const struct device *const dev = DEVICE_DT_GET(DT_NODELABEL(opt3001));
    if (dev == NULL || !device_is_ready(dev)) {
        debug_printk("Error: Light sensor not ready\n");
        return NULL;
    }
    return dev;
}

static const struct device *get_accel_sensor(void)
{
    const struct device *const dev = DEVICE_DT_GET(DT_NODELABEL(lis3dh));
    if (dev == NULL || !device_is_ready(dev)) {
        debug_printk("Error: Accelerometer not ready\n");
        return NULL;
    }
    return dev;
}

static void write_sensor_data(const sensor_readings_t *value)
{
    pthread_rwlock_wrlock(&g_sensor_buffer.lock);
    
    uint16_t idx = g_sensor_buffer.write_index;
    g_sensor_buffer.temp[idx] = value->temp;
    g_sensor_buffer.light[idx] = value->light;
    g_sensor_buffer.accel_x[idx] = value->accel_x;
    g_sensor_buffer.accel_y[idx] = value->accel_y;
    g_sensor_buffer.accel_z[idx] = value->accel_z;
    g_sensor_buffer.write_index = (idx + 1) % SENSOR_BUFFER_SIZE;
    
    pthread_rwlock_unlock(&g_sensor_buffer.lock);
}

static void update_environmental_state(const sensor_readings_t *value)
{
    bool was_braking = g_env_state.is_braking;
    bool was_dark = g_env_state.ambient_dark;
    
    /* Braking detection: 2 consecutive samples below threshold */
    double accel_z = sensor_value_to_double(&value->accel_z);
    bool current_sample_braking = (accel_z < BRAKING_ACCEL_THRESHOLD);
    
    if (current_sample_braking && prev_sample_braking && !was_braking) {
        g_env_state.previous_brightness = light_modes_get_current_pwm();
        g_env_state.is_braking = true;
        debug_printk("Braking detected: %.2f m/s^2\n", accel_z);
    } else if (!current_sample_braking && !prev_sample_braking && was_braking) {
        g_env_state.is_braking = false;
        debug_printk("Braking ended: %.2f m/s^2\n", accel_z);
    }
    
    prev_sample_braking = current_sample_braking;
    
    /* Ambient darkness detection with hysteresis */
    double light_lux = sensor_value_to_double(&value->light);
    
    if (was_dark) {
        if (light_lux >= AMBIENT_BRIGHT_THRESHOLD) {
            ambient_bright_sample_count++;
            ambient_dark_sample_count = 0;
            
            if (ambient_bright_sample_count >= AMBIENT_DARK_SAMPLES_REQUIRED) {
                g_env_state.ambient_dark = false;
                ambient_bright_sample_count = 0;
                debug_printk("Ambient light: BRIGHT (%.2f lux)\n", light_lux);
            }
        } else {
            ambient_bright_sample_count = 0;
        }
    } else {
        if (light_lux < AMBIENT_DARK_THRESHOLD) {
            ambient_dark_sample_count++;
            ambient_bright_sample_count = 0;
            
            if (ambient_dark_sample_count >= AMBIENT_DARK_SAMPLES_REQUIRED) {
                g_env_state.ambient_dark = true;
                ambient_dark_sample_count = 0;
                debug_printk("Ambient light: DARK (%.2f lux)\n", light_lux);
            }
        } else {
            ambient_dark_sample_count = 0;
        }
    }
}

static void sensor_data_collector(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    const struct device *const temp_sensor = get_temp_sensor();
    const struct device *const light_sensor = get_light_sensor();
    const struct device *const accel_sensor = get_accel_sensor();

    if (!temp_sensor && !light_sensor && !accel_sensor) {
        debug_printk("No sensors available\n");
        return;
    }

    while (threads_running) {
        sensor_readings_t value;
        
        if (temp_sensor) {
            sensor_sample_fetch(temp_sensor);
            sensor_channel_get(temp_sensor, SENSOR_CHAN_DIE_TEMP, &value.temp);
            debug_printk("Temp: %d.%06d C\n", value.temp.val1, value.temp.val2);
        }
        
        if (light_sensor) {
            sensor_sample_fetch(light_sensor);
            sensor_channel_get(light_sensor, SENSOR_CHAN_LIGHT, &value.light);
            debug_printk("Light: %d.%06d lux\n", value.light.val1, value.light.val2);
        }
        
        if (accel_sensor) {
            sensor_sample_fetch(accel_sensor);
            sensor_channel_get(accel_sensor, SENSOR_CHAN_ACCEL_X, &value.accel_x);
            sensor_channel_get(accel_sensor, SENSOR_CHAN_ACCEL_Y, &value.accel_y);
            sensor_channel_get(accel_sensor, SENSOR_CHAN_ACCEL_Z, &value.accel_z);
            debug_printk("Accel X:%d.%06d Y:%d.%06d Z:%d.%06d m/s^2\n",
                   value.accel_x.val1, value.accel_x.val2,
                   value.accel_y.val1, value.accel_y.val2,
                   value.accel_z.val1, value.accel_z.val2);
        }
        
        write_sensor_data(&value);
        update_environmental_state(&value);
        light_modes_update_smart_pwm();
        
        k_sleep(K_MSEC(TIME_SAMPLING_INTERVAL_MS));
    }
}

static void stationary_monitor_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);
    
    const double GRAVITY = 9.81;
    const double TOLERANCE = 0.10;
    const double MIN_MAGNITUDE = GRAVITY * (1.0 - TOLERANCE);
    const double MAX_MAGNITUDE = GRAVITY * (1.0 + TOLERANCE);
    const uint16_t SAMPLES_TO_CHECK = 300;
    
    /* Wait for initial buffer fill */
    k_sleep(K_SECONDS(150));
    
    while (threads_running) {
        debug_printk("Stationary check\n");
        
        pthread_rwlock_rdlock(&g_sensor_buffer.lock);
        
        bool is_stationary = true;
        uint16_t current_idx = g_sensor_buffer.write_index;
        
        for (uint16_t i = 0; i < SAMPLES_TO_CHECK; i++) {
            uint16_t idx = (current_idx + SENSOR_BUFFER_SIZE - 1 - i) % SENSOR_BUFFER_SIZE;
            
            double x = sensor_value_to_double(&g_sensor_buffer.accel_x[idx]);
            double y = sensor_value_to_double(&g_sensor_buffer.accel_y[idx]);
            double z = sensor_value_to_double(&g_sensor_buffer.accel_z[idx]);
            double magnitude = sqrt(x*x + y*y + z*z);
            
            if (magnitude < MIN_MAGNITUDE || magnitude > MAX_MAGNITUDE) {
                is_stationary = false;
                break;
            }
        }
        
        pthread_rwlock_unlock(&g_sensor_buffer.lock);
        
        if (is_stationary) {
            debug_printk("Stationary for 2.5 minutes, auto-off\n");
            main_state_machine_auto_off(true);
            k_msleep(100);
            power_management_enter_sleep();
        }
        
        k_sleep(K_SECONDS(150));
    }
}

void sensor_threads_start(void)
{
    if (threads_running) {
        debug_printk("Sensor threads already running\n");
        return;
    }
    
    debug_printk("Starting sensor threads\n");
    threads_running = true;
    
    /* Reset state variables */
    prev_sample_braking = false;
    ambient_dark_sample_count = 0;
    ambient_bright_sample_count = 0;
    
    sensor_thread_id = k_thread_create(&sensor_thread_data, sensor_stack,
                                      K_THREAD_STACK_SIZEOF(sensor_stack),
                                      sensor_data_collector,
                                      NULL, NULL, NULL,
                                      SENSOR_THREAD_PRIORITY, 0, K_NO_WAIT);
    
    stationary_thread_id = k_thread_create(&stationary_thread_data, stationary_stack,
                                          K_THREAD_STACK_SIZEOF(stationary_stack),
                                          stationary_monitor_thread,
                                          NULL, NULL, NULL,
                                          STATIONARY_THREAD_PRIORITY, 0, K_NO_WAIT);
}

void sensor_threads_stop(void)
{
    if (!threads_running) {
        debug_printk("Sensor threads not running\n");
        return;
    }
    
    debug_printk("Stopping sensor threads\n");
    threads_running = false;
    
    /* Give threads time to exit gracefully */
    k_msleep(100);
    
    if (sensor_thread_id) {
        k_thread_abort(sensor_thread_id);
        sensor_thread_id = NULL;
    }
    
    if (stationary_thread_id) {
        k_thread_abort(stationary_thread_id);
        stationary_thread_id = NULL;
    }
}
