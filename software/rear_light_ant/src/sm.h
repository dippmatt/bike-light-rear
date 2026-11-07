/*
 * Central state machine (SPEC sections 1 to 9 and the request rules of 10).
 *
 * The main loop feeds it one event at a time (it is the single consumer of
 * the event queue). It is the only writer of the system state and the only
 * caller of the output operations. It has no hardware access of its own:
 * everything goes through struct sm_ops, so the host tests can drive it.
 */

#ifndef SM_H
#define SM_H

#include <stdbool.h>
#include <stdint.h>

#include "app_events.h"
#include "battery_logic.h"

/* Numeric values are internal (SPEC SYS-1) */
enum system_state {
    LED_OFF = 0,         /* light off, system awake (ANT reachable) */
    LED_STEADY = 1,      /* solid base brightness */
    LED_FLASH = 2,       /* base brightness with periodic double flash */
    LED_SMART = 3,       /* sensor-controlled brightness */
    IDLE_CHARGING = 4,   /* light off, USB connected */
    SYSTEM_STATE_COUNT
};

struct sm_ops {
    /* Outputs */
    void (*light_set)(uint32_t pulse_usec);
    /* reachable_off: awake and Off, so a controller can still switch the light on */
    void (*status_set)(bool low_battery, bool charging, bool light_on, bool reachable_off);
    void (*status_off)(void);

    /* Timers and sensors that depend on the state */
    void (*flash_timer_start)(uint32_t ms); /* one-shot, posts EVT_FLASH_STEP */
    void (*flash_timer_stop)(void);
    void (*sensors_start)(void);
    void (*sensors_stop)(void);

    /* Battery: sample the voltage and run the thresholds; the tick timer
     * is started and stopped around Deep Sleep */
    enum battery_level (*battery_sample)(void);
    void (*battery_tick_start)(void);
    void (*battery_tick_stop)(void);

    /* Button */
    bool (*button_is_pressed)(void);
    int (*button_arm_wake)(void);
    int (*button_reenable)(void);

    /* Deep Sleep: stop and restart the radio side (ANT channel) */
    void (*sleep_enter)(void);
    void (*sleep_exit)(void);

    /* The published state changed (also called once at start-up) */
    void (*state_changed)(enum system_state state);

    uint32_t (*now_ms)(void);
    void (*delay_ms)(uint32_t ms);
    void (*reset)(void);
};

/**
 * Start the state machine in @p boot_state. Runs its entry action, so the
 * outputs behind @p ops must be initialised.
 */
void sm_init(const struct sm_ops *ops, enum system_state boot_state, bool usb_present);

/** Handle one event of the state machine kind (see app_events.h) */
void sm_handle_event(const struct app_event *evt);

/**
 * Boot policy (BOOT-1, BOOT-2): Idle Charging after a power-on reset with
 * USB power present, otherwise the first Active Mode.
 */
enum system_state sm_boot_state(bool power_on_reset, bool usb_present);

/** The state entered by a wake or boot that turns the light on */
enum system_state sm_first_active_mode(void);

enum system_state sm_current_state(void);

bool sm_is_sleeping(void);
bool sm_usb_present(void);
bool sm_low_battery(void);

#endif /* SM_H */
