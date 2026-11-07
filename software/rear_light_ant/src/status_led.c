/*
 * Status LED, see status_led.h.
 */

#include "status_led.h"

#include "board.h"
#include "config.h"
#include "sd_includes.h"
#include "soft_timer.h"

enum pattern {
    PATTERN_NONE,      /* steady level, no timer */
    PATTERN_BLINK,     /* toggled by a periodic timer */
    PATTERN_PULSE,     /* short pulse, a one-shot timer ends each phase */
};

/* The pattern that owns the LED: a blink event that is still queued after
 * the timer stopped must not toggle it */
static enum pattern pattern;
static bool pulse_lit;
static uint32_t pulse_on_ms;
static uint32_t pulse_period_ms;

/* Starts with a pulse, so the change is visible at once */
static void pulse_start(uint32_t on_ms, uint32_t period_ms)
{
    pattern = PATTERN_PULSE;
    pulse_on_ms = on_ms;
    pulse_period_ms = period_ms;
    pulse_lit = true;
    status_led_raw(true);
    soft_timer_start(ST_STATUS, on_ms, 0U);
}

void status_led_raw(bool on)
{
    if (on) {
        NRF_P0->OUTCLR = 1UL << PIN_STATUS_LED;
    } else {
        NRF_P0->OUTSET = 1UL << PIN_STATUS_LED;
    }
}

void status_led_init(void)
{
    NRF_P0->OUTSET = 1UL << PIN_STATUS_LED;
    NRF_P0->DIRSET = 1UL << PIN_STATUS_LED;
}

void status_led_set(bool low_battery, bool charging, bool light_on, bool reachable_off,
                    bool parked)
{
    /* Stop any pattern first so the timer cannot toggle a freshly set level */
    soft_timer_stop(ST_STATUS);
    pattern = PATTERN_NONE;

    if (low_battery) {
        pattern = PATTERN_BLINK;
        status_led_raw(true);
        soft_timer_start(ST_STATUS, BLINK_INTERVAL_LOW_BATTERY_MS, BLINK_INTERVAL_LOW_BATTERY_MS);
        return;
    }

    if (charging) {
        pattern = PATTERN_BLINK;
        status_led_raw(true);
        soft_timer_start(ST_STATUS, BLINK_INTERVAL_CHARGING_MS, BLINK_INTERVAL_CHARGING_MS);
        return;
    }

    if (parked) {
        pulse_start(PARKED_PULSE_ON_MS, PARKED_PULSE_PERIOD_MS);
        return;
    }

    if (!light_on && reachable_off) {
        pulse_start(HEARTBEAT_ON_MS, HEARTBEAT_PERIOD_MS);
        return;
    }

    status_led_raw(light_on);
}

void status_led_on_blink(void)
{
    switch (pattern) {
    case PATTERN_BLINK:
        NRF_P0->OUT ^= 1UL << PIN_STATUS_LED;
        break;
    case PATTERN_PULSE:
        pulse_lit = !pulse_lit;
        status_led_raw(pulse_lit);
        soft_timer_start(ST_STATUS, pulse_lit ? pulse_on_ms : pulse_period_ms - pulse_on_ms, 0U);
        break;
    case PATTERN_NONE:
    default:
        break;
    }
}

void status_led_off(void)
{
    soft_timer_stop(ST_STATUS);
    pattern = PATTERN_NONE;
    status_led_raw(false);
}
