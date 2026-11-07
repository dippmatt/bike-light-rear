/*
 * Main LED: PWM on P0.09, duty given in microseconds of the PWM period
 * (LIGHT_PWM_PERIOD_USEC). Same interface as light.h of the Zephyr firmware.
 */

#ifndef LIGHT_H
#define LIGHT_H

#include <stdint.h>

void light_init(void);

/**
 * Set the requested level. While a blank is active the PWM stays off and
 * the level takes effect when the blank ends (SMART-15).
 */
void light_set(uint32_t pulse_usec);

/** Switch the LED off for an ambient light measurement (SMART-12) */
void light_blank_begin(void);

/** Restore the level requested by the state machine, also a late one */
void light_blank_end(void);

#endif /* LIGHT_H */
