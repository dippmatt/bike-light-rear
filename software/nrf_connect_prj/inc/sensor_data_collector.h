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

#ifndef SENSOR_DATA_COLLECTOR_H
#define SENSOR_DATA_COLLECTOR_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <stdbool.h>
#include <pthread.h>

/* Sensor buffer size: 360 samples = 3 minutes at 500ms sampling rate */
#define SENSOR_BUFFER_SIZE 360

typedef struct {
    struct sensor_value temp; // internal nRF52833 temperature
    struct sensor_value light; // OPT3001 ambient light intensity
    struct sensor_value accel_x; // LIS3DH accelerometer X-axis
    struct sensor_value accel_y; // LIS3DH accelerometer Y-axis
    struct sensor_value accel_z; // LIS3DH accelerometer Z-axis
} sensor_readings_t;

/**
 * @brief Circular buffer for sensor history
 * 
 * Stores 360 samples (3 minutes at 500ms intervals) of all sensor data.
 * Protected by rwlock for thread-safe access.
 */
typedef struct {
    struct sensor_value temp[SENSOR_BUFFER_SIZE];
    struct sensor_value light[SENSOR_BUFFER_SIZE];
    struct sensor_value accel_x[SENSOR_BUFFER_SIZE];
    struct sensor_value accel_y[SENSOR_BUFFER_SIZE];
    struct sensor_value accel_z[SENSOR_BUFFER_SIZE];
    uint16_t write_index;  // Current write position in circular buffer
    pthread_rwlock_t lock;  // POSIX read-write lock for thread safety
} sensor_buffer_t;

/**
 * @brief Environmental state for SMART_MODE
 * 
 * Tracks current environmental conditions that affect LED behavior.
 */
typedef struct {
    bool is_braking;  // True when braking detected (z-accel < -2.0 m/s²)
    bool ambient_dark;  // True when ambient light < 30 lux
    uint32_t previous_brightness;  // Previous PWM pulse width in microseconds
} environmental_state_t;

/* Global sensor buffer and environmental state */
extern sensor_buffer_t g_sensor_buffer;
extern environmental_state_t g_env_state;

#endif /* SENSOR_DATA_COLLECTOR_H */