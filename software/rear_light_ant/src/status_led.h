/*
 * Status LED on P0.11 (active low), SPEC section 9.
 */

#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <stdbool.h>

void status_led_init(void);

/**
 * Show the status in priority order: low battery slow blink, charging fast
 * blink, Smart parked a very short pulse every few seconds, light on solid,
 * awake and Off a short pulse every few seconds (heartbeat), otherwise off
 * (STAT-1). Blinking is driven by the ST_STATUS timer; call
 * status_led_on_blink() for every EVT_STATUS_BLINK.
 */
void status_led_set(bool low_battery, bool charging, bool light_on, bool reachable_off,
                    bool parked);

void status_led_on_blink(void);

/** All off, blinking stopped */
void status_led_off(void);

/** Direct on/off, used when nothing else is running (error indication) */
void status_led_raw(bool on);

#endif /* STATUS_LED_H */
