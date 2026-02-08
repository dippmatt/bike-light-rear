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
#include <zephyr/sys/atomic.h>

#include "sensor_data_collector.h"
#include "utils.h"
#include "light_modes.h"
#include "main_state_machine.h"
#include "power_management.h"

#define SENSOR_THREAD_PRIORITY 7
#define SENSOR_THREAD_STACK_SIZE 2048
#define TIME_SAMPLING_INTERVAL_MS 500  /* Changed to 500ms for 360 samples = 3 minutes */

/* Braking detection thresholds */
#define BRAKING_ACCEL_THRESHOLD -3.0  /* m/s^2, negative z-axis for rear light */
#define AMBIENT_DARK_THRESHOLD 50.0   /* lux - threshold to enter dark mode */
#define AMBIENT_BRIGHT_THRESHOLD 150.0 /* lux - threshold to exit dark mode (hysteresis) */
#define AMBIENT_DARK_SAMPLES_REQUIRED 2  /* Number of consecutive samples required to change state */

/* Static variables for braking detection (need 2 consecutive samples) */
static bool prev_sample_braking = false;

/* Static variables for ambient light debouncing */
static uint8_t ambient_dark_sample_count = 0;
static uint8_t ambient_bright_sample_count = 0;

/* Sensor sampling control (shared between main thread and sensor thread) */
static atomic_t sensor_sampling_active = ATOMIC_INIT(0);
static struct k_thread *sensor_thread_handle = NULL;
K_SEM_DEFINE(sensor_sampling_sem, 0, 1);

/* Forward declarations */
static void write_sensor_data(const sensor_readings_t *value);
static void update_environmental_state(const sensor_readings_t *value);

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

/**
 * @brief Write sensor data to circular buffer
 * 
 * Thread-safe write using pthread rwlock.
 * 
 * @param value Sensor readings to write
 */
static void write_sensor_data(const sensor_readings_t *value)
{
    pthread_rwlock_wrlock(&g_sensor_buffer.lock);
    
    uint16_t idx = g_sensor_buffer.write_index;
    
    g_sensor_buffer.temp[idx] = value->temp;
    g_sensor_buffer.light[idx] = value->light;
    g_sensor_buffer.accel_x[idx] = value->accel_x;
    g_sensor_buffer.accel_y[idx] = value->accel_y;
    g_sensor_buffer.accel_z[idx] = value->accel_z;
    
    /* Increment write index (circular buffer) */
    g_sensor_buffer.write_index = (idx + 1) % SENSOR_BUFFER_SIZE;
    
    pthread_rwlock_unlock(&g_sensor_buffer.lock);
}

/**
 * @brief Detect and update environmental state
 * 
 * Updates global environmental state based on sensor readings.
 * Handles braking detection (2 consecutive samples) and ambient darkness.
 * 
 * @param value Current sensor readings
 */
static void update_environmental_state(const sensor_readings_t *value)
{
    bool was_braking = g_env_state.is_braking;
    bool was_dark = g_env_state.ambient_dark;
    
    /* Braking detection: z-acceleration < -2.0 m/s^2 for 2 consecutive samples */
    double accel_z = sensor_value_to_double(&value->accel_z);
    bool current_sample_braking = (accel_z < BRAKING_ACCEL_THRESHOLD);
    
    if (current_sample_braking && prev_sample_braking && !was_braking) {
        /* Entering braking - save current PWM state */
        g_env_state.previous_brightness = light_modes_get_current_pwm();
        g_env_state.is_braking = true;
        debug_printk("Braking detected: z-accel = %.2f m/s^2\n", accel_z);
    } else if (!current_sample_braking && !prev_sample_braking && was_braking) {
        /* Exiting braking - 2 consecutive samples above threshold */
        g_env_state.is_braking = false;
        debug_printk("Braking ended: z-accel = %.2f m/s^2\n", accel_z);
    }
    
    prev_sample_braking = current_sample_braking;
    
    /* Ambient darkness detection with debouncing and hysteresis to prevent feedback loop */
    double light_lux = sensor_value_to_double(&value->light);
    
    if (was_dark) {
        /* Currently dark - check if we should switch to bright */
        if (light_lux >= AMBIENT_BRIGHT_THRESHOLD) {
            ambient_bright_sample_count++;
            ambient_dark_sample_count = 0;  /* Reset dark counter */
            
            if (ambient_bright_sample_count >= AMBIENT_DARK_SAMPLES_REQUIRED) {
                g_env_state.ambient_dark = false;
                ambient_bright_sample_count = 0;  /* Reset after state change */
            }
        } else {
            /* Still dark - reset bright counter */
            ambient_bright_sample_count = 0;
        }
    } else {
        /* Currently bright - check if we should switch to dark */
        if (light_lux < AMBIENT_DARK_THRESHOLD) {
            ambient_dark_sample_count++;
            ambient_bright_sample_count = 0;  /* Reset bright counter */
            
            if (ambient_dark_sample_count >= AMBIENT_DARK_SAMPLES_REQUIRED) {
                g_env_state.ambient_dark = true;
                ambient_dark_sample_count = 0;  /* Reset after state change */
            }
        } else {
            /* Still bright - reset dark counter */
            ambient_dark_sample_count = 0;
        }
    }
    
    if (g_env_state.ambient_dark != was_dark) {
        debug_printk("Ambient light changed: %.2f lux (%s)\n", 
                     light_lux, g_env_state.ambient_dark ? "DARK" : "BRIGHT");
    }
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

    /* Store thread handle for suspend/resume control */
    sensor_thread_handle = k_current_get();

    while (1) {
        /* Wait until sampling is activated (block on semaphore) */
        k_sem_take(&sensor_sampling_sem, K_FOREVER);
        
        /* Sample continuously while active */
        while (atomic_get(&sensor_sampling_active)) {
            sensor_readings_t value = {0};
            
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
            
            /* Write sensor data to circular buffer */
            write_sensor_data(&value);
            
            /* Update environmental state (braking, darkness) */
            update_environmental_state(&value);
            
            /* Update PWM based on environmental state (we're only active in SMART_MODE) */
            light_modes_update_smart_pwm();
            
            k_sleep(K_MSEC(TIME_SAMPLING_INTERVAL_MS));
        }
    }
}

K_THREAD_DEFINE(sensor_data_collector_id, SENSOR_THREAD_STACK_SIZE, sensor_data_collector, NULL, 
        NULL, NULL, SENSOR_THREAD_PRIORITY, 0, 1000);

/**
 * @brief Start sensor data collection
 * 
 * Activates sensor sampling thread. Should only be called when entering
 * LED_SMART_MODE state.
 */
void sensor_data_collector_start(void)
{
    debug_printk("Sensor data collection start requested\n");
    if (!atomic_get(&sensor_sampling_active)) {
        atomic_set(&sensor_sampling_active, 1);
        debug_printk("Sensor data collection started\n");
        
        /* Give semaphore to wake up thread */
        k_sem_give(&sensor_sampling_sem);
    } else {
        debug_printk("Sensor data collection already active\n");
    }
}

/**
 * @brief Stop sensor data collection
 * 
 * Deactivates sensor sampling thread. Should be called when leaving
 * LED_SMART_MODE state.
 */
void sensor_data_collector_stop(void)
{
    if (atomic_get(&sensor_sampling_active)) {
        atomic_set(&sensor_sampling_active, 0);
        debug_printk("Sensor data collection stopped\n");
        
        /* Thread will check flag and stop sampling on next iteration */
    }
}

/**
 * @brief Stationary monitor thread
 * 
 * Checks every 60 seconds if the device has been stationary for 2.5 minutes.
 * If stationary and in SMART_MODE, automatically switches to OFF mode.
 */
void stationary_monitor_thread(void)
{
    /* Stationary detection using integer (milli) math to avoid double-precision
     * floating point on nRF52 which has no double FPU.
     * Compare squared magnitudes to avoid sqrt().
     *
     * Gravity ~= 9810 milli-m/s^2, 10% tolerance.
     * MIN = 8829, MAX = 10791 (milli-m/s^2)
     * MIN_SQ = 77,951,241, MAX_SQ = 116,445,681 (milli^2) */
    const int64_t MIN_MAGNITUDE_SQ = (int64_t)8829 * 8829;  /* (9.81 * 0.90)^2 in milli^2 */
    const int64_t MAX_MAGNITUDE_SQ = (int64_t)10791 * 10791; /* (9.81 * 1.10)^2 in milli^2 */
    const uint16_t SAMPLES_TO_CHECK = 300;  /* 2.5 minutes at 500ms sampling */
    
    /* Wait for initial buffer fill */
    k_sleep(K_SECONDS(150));  /* Wait 2.5 minutes for buffer to fill */
    
    while (1) {
        /* Check every 2.5 minutes for stationary state */
        debug_printk("Stationary monitor: checking for stationary state\n");
        
        /* Only check if in SMART_MODE */
        if (main_state_machine_get_state() != LED_SMART_MODE) {
            k_sleep(K_SECONDS(150));
            continue;
        }
        
        /* Acquire read lock to check sensor buffer */
        pthread_rwlock_rdlock(&g_sensor_buffer.lock);
        
        bool is_stationary = true;
        uint16_t current_idx = g_sensor_buffer.write_index;
        
#ifdef DEBUG
        int64_t min_mag_sq = INT64_MAX;
        int64_t max_mag_sq = 0;
        uint16_t samples_checked = 0;
        int64_t last_mag_sq = 0;
#endif
        
        /* Check last 300 samples (2.5 minutes) */
        for (uint16_t i = 0; i < SAMPLES_TO_CHECK; i++) {
            /* Calculate index going backwards in circular buffer */
            uint16_t idx = (current_idx + SENSOR_BUFFER_SIZE - 1 - i) % SENSOR_BUFFER_SIZE;
            
            /* Get acceleration values in milli-m/s^2 (integer) */
            int64_t x = sensor_value_to_milli(&g_sensor_buffer.accel_x[idx]);
            int64_t y = sensor_value_to_milli(&g_sensor_buffer.accel_y[idx]);
            int64_t z = sensor_value_to_milli(&g_sensor_buffer.accel_z[idx]);
            
            /* Calculate squared magnitude (avoids sqrt) */
            int64_t mag_sq = x * x + y * y + z * z;
            
#ifdef DEBUG
            last_mag_sq = mag_sq;
            samples_checked++;
            
            /* Track min/max */
            if (mag_sq < min_mag_sq) {
                min_mag_sq = mag_sq;
            }
            if (mag_sq > max_mag_sq) {
                max_mag_sq = mag_sq;
            }
#endif
            
            /* Check if within stationary range (squared comparison) */
            if (mag_sq < MIN_MAGNITUDE_SQ || mag_sq > MAX_MAGNITUDE_SQ) {
                is_stationary = false;
                break;
            }
        }
        
        pthread_rwlock_unlock(&g_sensor_buffer.lock);
        
#ifdef DEBUG
        /* Debug: Print magnitude calculation results */
        debug_printk("Stationary check: samples_checked=%u, is_stationary=%s, "
                     "min_mag_sq=%lld, max_mag_sq=%lld, last_mag_sq=%lld, "
                     "range=[%lld, %lld]\n",
                     samples_checked,
                     is_stationary ? "YES" : "NO",
                     min_mag_sq,
                     max_mag_sq,
                     last_mag_sq,
                     MIN_MAGNITUDE_SQ,
                     MAX_MAGNITUDE_SQ);
#endif
        
        /* If stationary for 2.5 minutes, trigger auto-off and enter sleep */
        if (is_stationary) {
            debug_printk("Stationary detected for 2.5 minutes, auto-off triggered\n");
            main_state_machine_auto_off();
            /* Small delay before entering sleep mode */
            k_msleep(100);
            /* Enter deep sleep mode - button press will wake the system */
            power_management_enter_sleep();
        }
        k_sleep(K_SECONDS(150));  /* Wait 2.5 minutes for buffer to fill again*/
    }
}

#define STATIONARY_THREAD_STACK_SIZE 2048
#define STATIONARY_THREAD_PRIORITY 8  /* Lower priority than sensor thread */

K_THREAD_DEFINE(stationary_monitor_id, STATIONARY_THREAD_STACK_SIZE, stationary_monitor_thread, 
        NULL, NULL, NULL, STATIONARY_THREAD_PRIORITY, 0, 150000);  /* Start after 150s */