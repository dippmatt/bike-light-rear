/*
 * Sensors for Smart mode: LIS3DH accelerometer and OPT3001 ambient light
 * sensor on I2C (SPEC section 6).
 *
 * While started, a timer (ST_SENSOR, SENSOR_TICK_MS) samples the sensors on
 * their own schedules and feeds the detectors, which post the edge events.
 * The ambient light measurement with the LED blanked is a sequence of
 * timer steps (ST_BLANK), so nothing blocks the main loop.
 */

#ifndef SENSORS_H
#define SENSORS_H

/**
 * Check and configure the sensors.
 * @return 0 if at least one sensor works, negative otherwise
 */
int sensors_init(void);

/** Start sampling (entering Smart); resets the detectors */
void sensors_start(void);

/** Stop sampling (leaving Smart); ends a running blank */
void sensors_stop(void);

/** EVT_SENSOR_TICK */
void sensors_on_tick(void);

/** EVT_BLANK_STEP */
void sensors_on_blank_step(void);

#endif /* SENSORS_H */
