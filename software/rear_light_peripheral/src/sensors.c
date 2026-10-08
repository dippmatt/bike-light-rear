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
#include <zephyr/sys/util.h>

#include "sensors.h"
#include "app_events.h"
#include "debug.h"

#define SENSOR_THREAD_PRIORITY   7
#define SENSOR_THREAD_STACK_SIZE 2048

/* --- Sampling ---------------------------------------------------------
 *
 * Sampling rate and detection timing are decoupled: every detector is
 * specified in milliseconds and converted to a sample count at compile
 * time (rounded up, so a time is never shorter than specified). Changing
 * a sampling interval therefore does not change any detection time.
 */

/* The accelerometer and the light sensor are sampled on their own schedule */
#define ACCEL_SAMPLING_INTERVAL_MS 50
#define LIGHT_SAMPLING_INTERVAL_MS 500

/* Thread wake-up period: the greatest common divisor of both intervals */
#define SENSOR_TICK_MS 50

#define ACCEL_TICKS (ACCEL_SAMPLING_INTERVAL_MS / SENSOR_TICK_MS)
#define LIGHT_TICKS (LIGHT_SAMPLING_INTERVAL_MS / SENSOR_TICK_MS)

BUILD_ASSERT(ACCEL_SAMPLING_INTERVAL_MS % SENSOR_TICK_MS == 0,
             "accelerometer interval must be a multiple of the sensor tick");
BUILD_ASSERT(LIGHT_SAMPLING_INTERVAL_MS % SENSOR_TICK_MS == 0,
             "light sensor interval must be a multiple of the sensor tick");

/* Accelerometer output data rate (Hz), pinned at init. Must be at least the
 * polling rate so every poll reads a fresh value. Supported by the driver:
 * 1, 10, 25, 50, 100, 200, 400 Hz. */
#define ACCEL_ODR_HZ 25

BUILD_ASSERT(ACCEL_ODR_HZ * ACCEL_SAMPLING_INTERVAL_MS >= 1000,
             "accelerometer output rate must not be below the polling rate");

#define MS_TO_SAMPLES(ms, interval_ms) DIV_ROUND_UP((ms), (interval_ms))

/* --- Braking ----------------------------------------------------------
 *
 * Z acceleration (Z along the direction of travel, positive = decelerating,
 * negative = accelerating) minus a slow baseline (removes the gravity component caused by mounting
 * tilt and slope), averaged over a short window, compared to a threshold.
 */

/* Braking threshold on the filtered Z acceleration (milli-m/s^2, i.e. +5.0 m/s^2) */
#define BRAKING_ACCEL_THRESHOLD_MILLI (5000)

/* Time the filtered value must stay above / at or below the threshold to start / end braking */
#define BRAKING_START_CONFIRM_MS 100
#define BRAKING_STOP_CONFIRM_MS  150

/* Extra time the brake light stays on after the braking phase has ended.
 * Added to the stop confirm: braking ends only after the filtered value has
 * stayed at or below the threshold for confirm + hold. Braking again within
 * that time keeps the light on without a gap. */
#define BRAKING_HOLD_MS 500

/* Window of the moving average applied before the threshold (single sample = 1 interval) */
#define BRAKING_SMOOTHING_MS 150

/* Time constant of the slow baseline; frozen while braking */
#define BRAKING_BASELINE_TAU_MS 8000

#define BRAKING_START_SAMPLES     MS_TO_SAMPLES(BRAKING_START_CONFIRM_MS, ACCEL_SAMPLING_INTERVAL_MS)
#define BRAKING_STOP_SAMPLES      MS_TO_SAMPLES(BRAKING_STOP_CONFIRM_MS + BRAKING_HOLD_MS, \
                                                ACCEL_SAMPLING_INTERVAL_MS)
#define BRAKING_SMOOTHING_SAMPLES MS_TO_SAMPLES(BRAKING_SMOOTHING_MS, ACCEL_SAMPLING_INTERVAL_MS)

BUILD_ASSERT(BRAKING_START_SAMPLES >= 1 && BRAKING_START_SAMPLES <= UINT8_MAX,
             "braking start confirm out of range");
BUILD_ASSERT(BRAKING_STOP_SAMPLES >= 1 && BRAKING_STOP_SAMPLES <= UINT8_MAX,
             "braking stop confirm plus hold out of range");
BUILD_ASSERT(BRAKING_SMOOTHING_SAMPLES >= 1 && BRAKING_SMOOTHING_SAMPLES <= UINT8_MAX,
             "braking smoothing window out of range");

/* The baseline is kept with extra fractional resolution so the slow
 * update does not stall on integer truncation */
#define BASELINE_SCALE 1024

/* --- Ambient light ----------------------------------------------------
 *
 * Hysteresis (milli-lux): enter dark below 50 lx, exit dark at or above
 * 150 lx, 2 consecutive light samples each.
 */
#define AMBIENT_DARK_THRESHOLD_MILLILUX   30000
#define AMBIENT_BRIGHT_THRESHOLD_MILLILUX 150000
#define AMBIENT_SAMPLES_REQUIRED          2

/* --- Stationary (auto-off) --------------------------------------------
 *
 * Integer (milli) math to avoid double-precision floating point on nRF52,
 * which has no double FPU. Squared magnitudes are compared to avoid sqrt().
 *
 * Gravity ~= 9810 milli-m/s^2, 100% tolerance (0g .. 2g):
 * MIN = 0, MAX = 19620 (milli-m/s^2)
 */
#define STATIONARY_MIN_MAGNITUDE_SQ ((int64_t)0 * 0)
#define STATIONARY_MAX_MAGNITUDE_SQ ((int64_t)19620 * 19620)

/* Time of uninterrupted stationary samples before auto-off (2.5 minutes) */
#define STATIONARY_TIMEOUT_MS 150000

#define STATIONARY_SAMPLES MS_TO_SAMPLES(STATIONARY_TIMEOUT_MS, ACCEL_SAMPLING_INTERVAL_MS)

BUILD_ASSERT(STATIONARY_SAMPLES >= 1 && STATIONARY_SAMPLES <= UINT16_MAX,
             "stationary timeout does not fit the sample counter");

static const struct device *const accel_sensor = DEVICE_DT_GET(DT_NODELABEL(lis3dh));
static const struct device *const light_sensor = DEVICE_DT_GET(DT_NODELABEL(opt3001));

static bool accel_ok;
static bool light_ok;

static atomic_t sampling_active = ATOMIC_INIT(0);
static K_SEM_DEFINE(sampling_sem, 0, 1);

/* Detector state; only touched by the sensor thread */
static bool braking;
static uint8_t braking_start_count;
static uint8_t braking_stop_count;

static bool baseline_valid;
static int64_t baseline_scaled; /* milli-m/s^2 * BASELINE_SCALE */

static int32_t smooth_buf[BRAKING_SMOOTHING_SAMPLES];
static uint8_t smooth_idx;
static uint8_t smooth_fill;
static int32_t smooth_sum;

static bool ambient_dark;
static uint8_t dark_sample_count;
static uint8_t bright_sample_count;

static uint16_t stationary_count;

static void detectors_reset(void)
{
    braking = false;
    braking_start_count = 0;
    braking_stop_count = 0;

    baseline_valid = false;
    baseline_scaled = 0;

    smooth_idx = 0;
    smooth_fill = 0;
    smooth_sum = 0;

    ambient_dark = false;
    dark_sample_count = 0;
    bright_sample_count = 0;

    stationary_count = 0;
}

/**
 * @brief Moving average over the last BRAKING_SMOOTHING_SAMPLES values
 *
 * Until the window is full the average covers the samples seen so far.
 */
static int32_t smooth(int32_t value)
{
    if (smooth_fill < BRAKING_SMOOTHING_SAMPLES) {
        smooth_buf[smooth_idx] = value;
        smooth_sum += value;
        smooth_fill++;
    } else {
        smooth_sum += value - smooth_buf[smooth_idx];
        smooth_buf[smooth_idx] = value;
    }

    smooth_idx = (smooth_idx + 1) % BRAKING_SMOOTHING_SAMPLES;

    return smooth_sum / smooth_fill;
}

/**
 * @brief Braking detection on baseline-corrected, smoothed Z acceleration
 *
 * The baseline is frozen while braking and while a start is being
 * confirmed, so a brake does not pull the baseline toward itself.
 */
static void process_braking(int64_t accel_z_milli)
{
    if (!baseline_valid) {
        /* First sample after entering SMART seeds the baseline */
        baseline_scaled = accel_z_milli * BASELINE_SCALE;
        baseline_valid = true;
    }

    int32_t linear_milli = (int32_t)(accel_z_milli - baseline_scaled / BASELINE_SCALE);
    int32_t smoothed_milli = smooth(linear_milli);
    bool decelerating = (smoothed_milli > BRAKING_ACCEL_THRESHOLD_MILLI);

    if (!braking) {
        if (decelerating) {
            braking_start_count++;
            if (braking_start_count >= BRAKING_START_SAMPLES) {
                braking = true;
                braking_start_count = 0;
                braking_stop_count = 0;
                debug_printk("Braking detected: filtered z-accel = %d milli-m/s^2\n",
                             (int)smoothed_milli);
                app_event_post(EVT_BRAKE_START, 0);
            }
        } else {
            braking_start_count = 0;
        }
    } else {
        if (!decelerating) {
            braking_stop_count++;
            if (braking_stop_count >= BRAKING_STOP_SAMPLES) {
                braking = false;
                braking_stop_count = 0;
                debug_printk("Braking ended: filtered z-accel = %d milli-m/s^2\n",
                             (int)smoothed_milli);
                app_event_post(EVT_BRAKE_STOP, 0);
            }
        } else {
            braking_stop_count = 0;
        }
    }

    /* Slow baseline: first-order low-pass, frozen while braking or while a
     * braking start is pending */
    if (!braking && braking_start_count == 0) {
        int64_t target_scaled = accel_z_milli * BASELINE_SCALE;

        baseline_scaled += (target_scaled - baseline_scaled) * ACCEL_SAMPLING_INTERVAL_MS /
                           (BRAKING_BASELINE_TAU_MS + ACCEL_SAMPLING_INTERVAL_MS);
    }
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
            debug_printk("Stationary for %d ms - requesting auto-off\n",
                         STATIONARY_TIMEOUT_MS);
            app_event_post(EVT_STATIONARY_TIMEOUT, 0);
        }
    } else {
        stationary_count = 0;
    }
}

static void sample_accel(void)
{
    struct sensor_value ax, ay, az;

    if (!accel_ok) {
        return;
    }

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

static void sample_light(void)
{
    struct sensor_value lux;

    if (!light_ok) {
        return;
    }

    if (sensor_sample_fetch(light_sensor) == 0 &&
        sensor_channel_get(light_sensor, SENSOR_CHAN_LIGHT, &lux) == 0) {
        process_ambient(sensor_value_to_milli(&lux));
    } else {
        debug_printk("Light sensor read failed\n");
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

        /* Both sensors are sampled immediately on activation, then each on
         * its own schedule */
        uint32_t accel_phase = 0;
        uint32_t light_phase = 0;

        while (atomic_get(&sampling_active)) {
            if (accel_phase == 0) {
                sample_accel();
            }
            if (light_phase == 0) {
                sample_light();
            }

            accel_phase = (accel_phase + 1) % ACCEL_TICKS;
            light_phase = (light_phase + 1) % LIGHT_TICKS;

            k_msleep(SENSOR_TICK_MS);
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
    } else {
        /* Pin the output data rate instead of relying on the driver default */
        struct sensor_value odr = { .val1 = ACCEL_ODR_HZ, .val2 = 0 };
        int ret = sensor_attr_set(accel_sensor, SENSOR_CHAN_ACCEL_XYZ,
                                  SENSOR_ATTR_SAMPLING_FREQUENCY, &odr);

        if (ret != 0) {
            debug_printk("Accelerometer output rate not set (%d), using driver default\n",
                         ret);
        }
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
