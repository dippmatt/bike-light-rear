/*
 * Software timers and uptime on RTC1 (RTC0 belongs to the SoftDevice).
 *
 * One compare channel serves all timers. Expiry only posts the timer's
 * event to the central queue, so no application logic runs in the
 * interrupt. Start and stop may be called from the main loop only.
 */

#ifndef SOFT_TIMER_H
#define SOFT_TIMER_H

#include <stdbool.h>
#include <stdint.h>

enum soft_timer_id {
    ST_FLASH = 0,   /* EVT_FLASH_STEP */
    ST_BUTTON,      /* EVT_BUTTON_SAMPLE */
    ST_BATTERY,     /* EVT_BATTERY_TICK */
    ST_STATUS,      /* EVT_STATUS_BLINK */
    ST_USB,         /* EVT_USB_POLL */
    ST_SENSOR,      /* EVT_SENSOR_TICK */
    ST_BLANK,       /* EVT_BLANK_STEP */
    ST_COUNT
};

/** Start RTC1; the SoftDevice must be enabled so that the LFCLK runs. */
void soft_timer_init(void);

/**
 * @param period_ms 0 for a one-shot timer, otherwise the repeat interval
 */
void soft_timer_start(enum soft_timer_id id, uint32_t delay_ms, uint32_t period_ms);

void soft_timer_stop(enum soft_timer_id id);

/** Milliseconds since soft_timer_init(); wraps after about 49 days */
uint32_t soft_timer_uptime_ms(void);

#endif /* SOFT_TIMER_H */
