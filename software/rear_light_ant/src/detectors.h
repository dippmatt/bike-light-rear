/*
 * Smart mode detectors (SPEC section 6): braking, ambient light and
 * stationary, on samples supplied by the sensor driver. Pure logic, no
 * hardware: edges are posted to the central event queue.
 *
 *   EVT_BRAKE_START / EVT_BRAKE_STOP
 *   EVT_AMBIENT_DARK / EVT_AMBIENT_BRIGHT
 *   EVT_STATIONARY_TIMEOUT
 */

#ifndef DETECTORS_H
#define DETECTORS_H

#include <stdbool.h>
#include <stdint.h>

/** Forget all detector state (entering Smart) */
void detectors_reset(void);

/** One accelerometer sample in milli-m/s^2 (Z along the direction of travel) */
void detectors_accel_sample(int64_t x_milli, int64_t y_milli, int64_t z_milli);

/** One ambient light measurement, free of the main LED's light, in milli-lux */
void detectors_ambient_sample(int64_t millilux);

/** How the sensor driver should measure the light next (SMART-12, SMART-13) */
enum light_plan {
    LIGHT_PLAN_SKIP,    /* no measurement now (braking, or a braking start is pending) */
    LIGHT_PLAN_READ,    /* LED is off: read the continuous conversion */
    LIGHT_PLAN_BLANKED, /* LED is lit: blank it for one conversion */
};

enum light_plan detectors_light_plan(void);

/**
 * Sensor ticks until the next light measurement is due.
 * @param measured true if the last measurement produced a value
 */
uint32_t detectors_light_next_ticks(bool measured);

bool detectors_braking(void);
bool detectors_ambient_dark(void);

#endif /* DETECTORS_H */
