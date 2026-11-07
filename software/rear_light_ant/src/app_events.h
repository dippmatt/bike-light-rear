/*
 * Central event queue of the application (SPEC SYS-3, SYS-4).
 *
 * Every input source posts events; the main loop is the single consumer and
 * the only place that changes state. Posting is safe from any interrupt.
 * If the queue is full the event is dropped and counted: all sources are
 * periodic or re-detecting, so a dropped event recurs.
 *
 * Driver events (EVT_SD and later) are coalesced: at most one of each type
 * waits in the queue, so a burst cannot crowd out state machine events.
 */

#ifndef APP_EVENTS_H
#define APP_EVENTS_H

#include <stdbool.h>
#include <stdint.h>

enum app_event_type {
    /* Handled by the state machine */
    EVT_BUTTON_SHORT,       /* button released before the long-press threshold */
    EVT_BUTTON_LONG,        /* button held for >= 1 s */
    EVT_REMOTE_SET_STATE,   /* arg = requested system_state, range-checked by the state machine */
    EVT_FLASH_STEP,         /* flash-pattern timer tick (only used in Flash) */
    EVT_BATTERY_TICK,       /* battery sampling tick */
    EVT_USB_CONNECTED,      /* VBUS appeared (edge) */
    EVT_USB_DISCONNECTED,   /* VBUS vanished (edge) */
    EVT_BRAKE_START,
    EVT_BRAKE_STOP,
    EVT_AMBIENT_DARK,
    EVT_AMBIENT_BRIGHT,
    EVT_STATIONARY_TIMEOUT,
    EVT_BUTTON_WAKE,        /* button pressed while in Deep Sleep */
    EVT_MOTION_DETECTED,    /* motion while Smart is parked (SMART-17) */

    /* Driver events handled in main.c (coalesced, never reach the state machine) */
    EVT_SD,                 /* SoftDevice event pending */
    EVT_BUTTON_EDGE,        /* button sense interrupt */
    EVT_BUTTON_SAMPLE,      /* press-length sampling timer */
    EVT_STATUS_BLINK,       /* status LED blink timer */
    EVT_USB_POLL,           /* VBUS poll timer */
    EVT_SENSOR_TICK,        /* sensor sampling timer */
    EVT_BLANK_STEP,         /* next step of the ambient light blank */

    EVT_TYPE_COUNT
};

#define EVT_FIRST_DRIVER_EVENT EVT_SD

struct app_event {
    uint8_t type; /* enum app_event_type */
    uint8_t arg;  /* only used by EVT_REMOTE_SET_STATE */
};

/** @return true if queued, false if dropped */
bool app_event_post(uint8_t type, uint8_t arg);

/** @return true and fill @p evt if an event was waiting */
bool app_event_get(struct app_event *evt);

/** @return true if an event is waiting (checked with interrupts masked before sleeping) */
bool app_event_pending(void);

/** Events dropped because the queue was full */
uint32_t app_event_dropped(void);

/** Empty the queue and clear the counter (tests, boot) */
void app_event_reset(void);

#endif /* APP_EVENTS_H */
