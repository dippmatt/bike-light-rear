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

#include "sensors.h"
#include "app_events.h"
#include "debug.h"

#define SENSOR_THREAD_PRIORITY   7
#define SENSOR_THREAD_STACK_SIZE 2048
#define SAMPLING_INTERVAL_MS     500

/* Braking: z-acceleration below this for 2 consecutive samples (milli-m/s^2) */
#define BRAKING_ACCEL_THRESHOLD_MILLI (-6000)

/* Ambient light hysteresis (milli-lux): enter dark below 50 lx,
 * exit dark at or above 150 lx, 2 consecutive samples each */
#define AMBIENT_DARK_THRESHOLD_MILLILUX   50000
#define AMBIENT_BRIGHT_THRESHOLD_MILLILUX 150000
#define AMBIENT_SAMPLES_REQUIRED          2

/* Stationary detection using integer (milli) math to avoid double-precision
 * floating point on nRF52 which has no double FPU.
 * Compare squared magnitudes to avoid sqrt().
 *
 * Gravity ~= 9810 milli-m/s^2, 10% tolerance:
 * MIN = 8829, MAX = 10791 (milli-m/s^2) */
#define STATIONARY_MIN_MAGNITUDE_SQ ((int64_t)8829 * 8829)
#define STATIONARY_MAX_MAGNITUDE_SQ ((int64_t)10791 * 10791)
/* 2.5 minutes of consecutive stationary samples at 500 ms */
#define STATIONARY_SAMPLES 300

static const struct device *const accel_sensor = DEVICE_DT_GET(DT_NODELABEL(lis3dh));
static const struct device *const light_sensor = DEVICE_DT_GET(DT_NODELABEL(opt3001));

static bool accel_ok;
static bool light_ok;

static atomic_t sampling_active = ATOMIC_INIT(0);
static K_SEM_DEFINE(sampling_sem, 0, 1);

/* Detector state; only touched by the sensor thread */
static bool braking;
static bool prev_sample_braking;
static bool ambient_dark;
static uint8_t dark_sample_count;
static uint8_t bright_sample_count;
static uint16_t stationary_count;

static void detectors_reset(void)
{
    braking = false;
    prev_sample_braking = false;
    ambient_dark = false;
    dark_sample_count = 0;
    bright_sample_count = 0;
    stationary_count = 0;
}

/**
 * @brief Braking detection: 2 consecutive samples below/above threshold
 */
static void process_braking(int64_t accel_z_milli)
{
    bool current_sample_braking = (accel_z_milli < BRAKING_ACCEL_THRESHOLD_MILLI);

    if (current_sample_braking && prev_sample_braking && !braking) {
        braking = true;
        debug_printk("Braking detected: z-accel = %lld milli-m/s^2\n", accel_z_milli);
        app_event_post(EVT_BRAKE_START, 0);
    } else if (!current_sample_braking && !prev_sample_braking && braking) {
        braking = false;
        debug_printk("Braking ended: z-accel = %lld milli-m/s^2\n", accel_z_milli);
        app_event_post(EVT_BRAKE_STOP, 0);
    }

    prev_sample_braking = current_sample_braking;
}

/**
 * @brief Ambient light detection with debouncing and hysteresis
 */
static void process_ambient(int64_t light_millilux)
{
    if (ambient_dark) {
        /* Currently dark - check if we should switch to bright */
        if (light_millilux >= AMBIENT_BRIGHT_THRESHOLD_MILLILUX) {
            bright_sample_count++;
            dark_sample_count = 0;
            if (bright_sample_count >= AMBIENT_SAMPLES_REQUIRED) {
                ambient_dark = false;
                bright_sample_count = 0;
                debug_printk("Ambient light changed: %lld milli-lux (BRIGHT)\n",
                             light_millilux);
                app_event_post(EVT_AMBIENT_BRIGHT, 0);
            }
        } else {
            bright_sample_count = 0;
        }
    } else {
        /* Currently bright - check if we should switch to dark */
        if (light_millilux < AMBIENT_DARK_THRESHOLD_MILLILUX) {
            dark_sample_count++;
            bright_sample_count = 0;
            if (dark_sample_count >= AMBIENT_SAMPLES_REQUIRED) {
                ambient_dark = true;
                dark_sample_count = 0;
                debug_printk("Ambient light changed: %lld milli-lux (DARK)\n",
                             light_millilux);
                app_event_post(EVT_AMBIENT_DARK, 0);
            }
        } else {
            dark_sample_count = 0;
        }
    }
}

/**
 * @brief Stationary detection: consecutive samples with |accel| ~ gravity
 *
 * A counter of consecutive stationary samples is equivalent to "all of the
 * last N samples stationary" - no sample buffer needed.
 */
static void process_stationary(int64_t x_milli, int64_t y_milli, int64_t z_milli)
{
    int64_t mag_sq = x_milli * x_milli + y_milli * y_milli + z_milli * z_milli;

    if (mag_sq >= STATIONARY_MIN_MAGNITUDE_SQ && mag_sq <= STATIONARY_MAX_MAGNITUDE_SQ) {
        stationary_count++;
        if (stationary_count >= STATIONARY_SAMPLES) {
            stationary_count = 0;
            debug_printk("Stationary for 2.5 minutes - requesting auto-off\n");
            app_event_post(EVT_STATIONARY_TIMEOUT, 0);
        }
    } else {
        stationary_count = 0;
    }
}

static void sample_once(void)
{
    if (accel_ok) {
        struct sensor_value ax, ay, az;

        if (sensor_sample_fetch(accel_sensor) == 0 &&
            sensor_channel_get(accel_sensor, SENSOR_CHAN_ACCEL_X, &ax) == 0 &&
            sensor_channel_get(accel_sensor, SENSOR_CHAN_ACCEL_Y, &ay) == 0 &&
            sensor_channel_get(accel_sensor, SENSOR_CHAN_ACCEL_Z, &az) == 0) {
            int64_t x_milli = sensor_value_to_milli(&ax);
            int64_t y_milli = sensor_value_to_milli(&ay);
            int64_t z_milli = sensor_value_to_milli(&az);

            process_braking(z_milli);
            process_stationary(x_milli, y_milli, z_milli);
        } else {
            debug_printk("Accelerometer read failed\n");
        }
    }

    if (light_ok) {
        struct sensor_value lux;

        if (sensor_sample_fetch(light_sensor) == 0 &&
            sensor_channel_get(light_sensor, SENSOR_CHAN_LIGHT, &lux) == 0) {
            process_ambient(sensor_value_to_milli(&lux));
        } else {
            debug_printk("Light sensor read failed\n");
        }
    }
}

static void sensor_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (1) {
        /* Block until sampling is activated */
        k_sem_take(&sampling_sem, K_FOREVER);

        while (atomic_get(&sampling_active)) {
            sample_once();
            k_sleep(K_MSEC(SAMPLING_INTERVAL_MS));
        }
    }
}

K_THREAD_DEFINE(sensor_thread_id, SENSOR_THREAD_STACK_SIZE, sensor_thread,
                NULL, NULL, NULL, SENSOR_THREAD_PRIORITY, 0, 0);

int sensors_init(void)
{
    accel_ok = device_is_ready(accel_sensor);
    if (!accel_ok) {
        debug_printk("Error: accelerometer %s not ready\n", accel_sensor->name);
    }

    light_ok = device_is_ready(light_sensor);
    if (!light_ok) {
        debug_printk("Error: light sensor %s not ready\n", light_sensor->name);
    }

    return (accel_ok || light_ok) ? 0 : -ENODEV;
}

void sensors_start(void)
{
    if (!atomic_get(&sampling_active)) {
        detectors_reset();
        atomic_set(&sampling_active, 1);
        k_sem_give(&sampling_sem);
        debug_printk("Sensor sampling started\n");
    }
}

void sensors_stop(void)
{
    if (atomic_get(&sampling_active)) {
        atomic_set(&sampling_active, 0);
        debug_printk("Sensor sampling stopped\n");
    }
}
