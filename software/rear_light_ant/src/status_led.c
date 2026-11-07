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
    PATTERN_HEARTBEAT, /* short pulse, a one-shot timer ends each phase */
};

/* The pattern that owns the LED: a blink event that is still queued after
 * the timer stopped must not toggle it */
static enum pattern pattern;
static bool heartbeat_lit;

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

void status_led_set(bool low_battery, bool charging, bool light_on, bool reachable_off)
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

    if (!light_on && reachable_off) {
        /* Starts with a pulse, so the change is visible at once */
        pattern = PATTERN_HEARTBEAT;
        heartbeat_lit = true;
        status_led_raw(true);
        soft_timer_start(ST_STATUS, HEARTBEAT_ON_MS, 0U);
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
    case PATTERN_HEARTBEAT:
        heartbeat_lit = !heartbeat_lit;
        status_led_raw(heartbeat_lit);
        soft_timer_start(ST_STATUS,
                         heartbeat_lit ? HEARTBEAT_ON_MS : HEARTBEAT_PERIOD_MS - HEARTBEAT_ON_MS,
                         0U);
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
