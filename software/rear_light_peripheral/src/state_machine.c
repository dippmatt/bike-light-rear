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

#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/printk.h>

#include "state_machine.h"
#include "app_events.h"
#include "light.h"
#include "status_led.h"
#include "button.h"
#include "sensors.h"
#include "battery.h"
#include "power.h"
#include "debug.h"

/* External state writes (BLE) are ignored while the local state changed
 * less than this long ago, so a remote cannot fight a just-pressed button. */
#define BLE_STABLE_WINDOW_MS 3000

/* Selection window: after the light goes from an inactive state (Deep Sleep,
 * Off, Idle Charging) to an Active Mode, short presses cycle through the
 * Active Modes only for this many battery ticks. Afterwards a short press
 * leaves the Active Modes for the end state of the cycle (Idle Charging or
 * Deep Sleep).
 *
 * The window is counted in battery ticks (the existing free-running 5 s
 * sampling tick) instead of a dedicated timer. The tick phase is random
 * relative to the switch-on, so 4 ticks last between 15 and 20 s. */
#define MODE_SELECT_WINDOW_BATTERY_TICKS 4

/* FLASH mode: double flash every 1.5 s
 * (peak 70 ms, base 70 ms, peak 70 ms, base for the rest of the cycle) */
#define FLASH_CYCLE_MS 1500
#define FLASH_STEP_MS  70
#define FLASH_REST_MS  (FLASH_CYCLE_MS - 3 * FLASH_STEP_MS)

/* Maximum time to wait for button release before entering System OFF.
 * Entering System OFF with the wake level already active would wake
 * (reset) the system immediately. */
#define POWEROFF_RELEASE_WAIT_MS   10000
#define POWEROFF_RELEASE_POLL_MS   20

/* Mode Cycle: the Active Modes in short-press order. This is the only place
 * that knows the order - the end of the cycle (Idle Charging or Deep Sleep)
 * and the entry point (first mode) are derived from it. */
static const enum system_state mode_cycle[] = {
    LED_SMART_MODE,
    LED_50_PERCENT,
    LED_50_80_FLASH,
};

#define MODE_CYCLE_LEN ARRAY_SIZE(mode_cycle)

/* All of the following is owned exclusively by the state machine thread. */
static enum system_state state;
static atomic_t published_state = ATOMIC_INIT(LED_OFF);
static int64_t last_change_ms;

/* Battery ticks since the light was last switched on from an inactive state;
 * saturates at MODE_SELECT_WINDOW_BATTERY_TICKS (window closed) */
static uint8_t selection_ticks = MODE_SELECT_WINDOW_BATTERY_TICKS;

static bool usb_present;
static bool low_battery;
static bool braking;
static bool ambient_dark;
static uint8_t flash_phase;

static struct k_timer flash_timer;

/* Timer ISR only posts an event; the PWM write happens in the SM thread */
static void flash_timer_expiry(struct k_timer *timer)
{
    ARG_UNUSED(timer);
    app_event_post(EVT_FLASH_STEP, 0);
}

/* Position of a state in the Mode Cycle, or -1 if it is not an Active Mode */
static int cycle_index(enum system_state s)
{
    for (size_t i = 0; i < MODE_CYCLE_LEN; i++) {
        if (mode_cycle[i] == s) {
            return (int)i;
        }
    }

    return -1;
}

static bool light_is_on(enum system_state s)
{
    return cycle_index(s) >= 0;
}

static void status_refresh(void)
{
    status_led_set(low_battery, usb_present, light_is_on(state));
}

/* SMART mode brightness priority: braking > ambient dark > off */
static void apply_smart_pwm(void)
{
    if (braking) {
        light_set(LIGHT_PULSE_PEAK);
    } else if (ambient_dark) {
        light_set(LIGHT_PULSE_BASE);
    } else {
        light_set(LIGHT_PULSE_OFF);
    }
}

static void state_enter(enum system_state s)
{
    switch (s) {
    case LED_OFF:
        light_set(LIGHT_PULSE_OFF);
        break;
    case LED_50_PERCENT:
        light_set(LIGHT_PULSE_BASE);
        break;
    case LED_50_80_FLASH:
        light_set(LIGHT_PULSE_BASE);
        flash_phase = 0;
        k_timer_start(&flash_timer, K_MSEC(FLASH_CYCLE_MS), K_NO_WAIT);
        break;
    case LED_SMART_MODE:
        braking = false;
        ambient_dark = false;
        light_set(LIGHT_PULSE_OFF);
        sensors_start();
        break;
    case IDLE_CHARGING:
        light_set(LIGHT_PULSE_OFF);
        break;
    }
}

static void state_exit(enum system_state s)
{
    switch (s) {
    case LED_50_80_FLASH:
        k_timer_stop(&flash_timer);
        break;
    case LED_SMART_MODE:
        sensors_stop();
        break;
    default:
        break;
    }
}

static void transition(enum system_state to)
{
    if (to == state) {
        return;
    }

    debug_printk("State transition: %d -> %d\n", state, to);

    bool was_active = light_is_on(state);

    state_exit(state);
    state = to;
    state_enter(to);

    last_change_ms = k_uptime_get();

    /* Only an inactive -> Active Mode transition opens the selection window;
     * changes between Active Modes neither open nor restart it */
    if (!was_active && light_is_on(to)) {
        selection_ticks = 0;
    }

    atomic_set(&published_state, to);
    status_refresh();
}

/**
 * @brief Shut everything down and enter System OFF (deep sleep)
 *
 * Called only from the state machine thread, synchronously - there is no
 * delayed sleep work that a button press could race against. If the button
 * wake configuration fails, the device stays awake in LED_OFF with a
 * working button instead of ending up with dead inputs.
 */
static void do_poweroff(void)
{
    debug_printk("Powering off...\n");

    state_exit(state); /* stops flash timer / sensor sampling */
    light_set(LIGHT_PULSE_OFF);
    status_led_off();

    /* Wait for button release: the long press that triggered this fires
     * while the button is still held, and System OFF would wake (reset)
     * immediately with the wake level already active. A genuinely stuck
     * button proceeds after the timeout and causes a visible wake+reboot
     * instead of a dead device. */
    for (int i = 0; i < POWEROFF_RELEASE_WAIT_MS / POWEROFF_RELEASE_POLL_MS &&
                    button_is_pressed(); i++) {
        k_msleep(POWEROFF_RELEASE_POLL_MS);
    }

    if (button_prepare_wake() != 0) {
        /* Recover instead of sleeping with a dead button: restore edge
         * interrupts and stay awake in LED_OFF so the user can retry. */
        debug_printk("Wake configuration failed - staying awake in LED_OFF\n");
        (void)button_reenable();
        state = LED_OFF;
        last_change_ms = k_uptime_get();
        atomic_set(&published_state, LED_OFF);
        status_refresh();
        return;
    }

    power_off_finalize(); /* does not return */
}

static void handle_button_short(void)
{
    int idx = cycle_index(state);

    if (idx < 0) {
        /* Off or Idle Charging: enter the first Active Mode */
        transition(mode_cycle[0]);
    } else if (selection_ticks < MODE_SELECT_WINDOW_BATTERY_TICKS &&
               (size_t)idx + 1 < MODE_CYCLE_LEN) {
        /* Inside the selection window: next Active Mode */
        transition(mode_cycle[idx + 1]);
    } else if (usb_present) {
        /* After the last Active Mode, or after the selection window closed,
         * with USB power present */
        transition(IDLE_CHARGING);
    } else {
        do_poweroff();
    }
}

static void handle_ble_set_state(uint8_t requested)
{
    /* Only Off and the Active Modes can be requested; Idle Charging is set
     * by USB power alone. Invalid requests are ignored. */
    if (requested != LED_OFF && cycle_index((enum system_state)requested) < 0) {
        printk("Control ignored (not requestable): 0x%02x\n", requested);
        return;
    }

    if (requested == (uint8_t)state) {
        return;
    }

    if (k_uptime_get() - last_change_ms < BLE_STABLE_WINDOW_MS) {
        printk("Control ignored (local change < %d ms): 0x%02x\n",
               BLE_STABLE_WINDOW_MS, requested);
        return;
    }

    printk("Control value changed: 0x%02x\n", requested);
    /* A BLE-written LED_OFF keeps the system awake (BLE stays reachable);
     * only the button and the auto-off paths power the system down. */
    transition((enum system_state)requested);
}

static void handle_flash_step(void)
{
    if (state != LED_50_80_FLASH) {
        return; /* stale event from a stopped timer */
    }

    switch (flash_phase) {
    case 0:
        light_set(LIGHT_PULSE_PEAK);
        flash_phase = 1;
        k_timer_start(&flash_timer, K_MSEC(FLASH_STEP_MS), K_NO_WAIT);
        break;
    case 1:
        light_set(LIGHT_PULSE_BASE);
        flash_phase = 2;
        k_timer_start(&flash_timer, K_MSEC(FLASH_STEP_MS), K_NO_WAIT);
        break;
    case 2:
        light_set(LIGHT_PULSE_PEAK);
        flash_phase = 3;
        k_timer_start(&flash_timer, K_MSEC(FLASH_STEP_MS), K_NO_WAIT);
        break;
    default:
        light_set(LIGHT_PULSE_BASE);
        flash_phase = 0;
        k_timer_start(&flash_timer, K_MSEC(FLASH_REST_MS), K_NO_WAIT);
        break;
    }
}

static void handle_battery_tick(void)
{
    /* The battery tick also times the selection window */
    if (selection_ticks < MODE_SELECT_WINDOW_BATTERY_TICKS) {
        selection_ticks++;
    }

    enum battery_level level = battery_sample_and_process();

    if (level == BATTERY_CRITICAL) {
        debug_printk("Battery critically low (%u mV) - powering off\n",
                     battery_last_mv());
        do_poweroff();
        return;
    }

    bool low = (level == BATTERY_LOW);
    if (low != low_battery) {
        debug_printk("Low battery state: %s (%u mV)\n",
                     low ? "on" : "off", battery_last_mv());
        low_battery = low;
        status_refresh();
    }
}

static void handle_event(const struct app_event *evt)
{
    switch (evt->type) {
    case EVT_BUTTON_SHORT:
        handle_button_short();
        break;
    case EVT_BUTTON_LONG:
        do_poweroff();
        break;
    case EVT_BLE_SET_STATE:
        handle_ble_set_state(evt->arg);
        break;
    case EVT_FLASH_STEP:
        handle_flash_step();
        break;
    case EVT_BATTERY_TICK:
        handle_battery_tick();
        break;
    case EVT_USB_CONNECTED:
        usb_present = true;
        status_refresh();
        break;
    case EVT_USB_DISCONNECTED:
        usb_present = false;
        if (state == IDLE_CHARGING) {
            debug_printk("USB disconnected in IDLE_CHARGING - powering off\n");
            do_poweroff();
        } else {
            status_refresh();
        }
        break;
    case EVT_BRAKE_START:
        if (state == LED_SMART_MODE) {
            braking = true;
            apply_smart_pwm();
        }
        break;
    case EVT_BRAKE_STOP:
        if (state == LED_SMART_MODE) {
            braking = false;
            apply_smart_pwm();
        }
        break;
    case EVT_AMBIENT_DARK:
        if (state == LED_SMART_MODE) {
            ambient_dark = true;
            apply_smart_pwm();
        }
        break;
    case EVT_AMBIENT_BRIGHT:
        if (state == LED_SMART_MODE) {
            ambient_dark = false;
            apply_smart_pwm();
        }
        break;
    case EVT_STATIONARY_TIMEOUT:
        if (state == LED_SMART_MODE) {
            debug_printk("Stationary timeout in SMART mode - powering off\n");
            do_poweroff();
        }
        break;
    default:
        debug_printk("Unknown event type: %u\n", evt->type);
        break;
    }
}

void sm_init(enum system_state boot_state, bool usb)
{
    k_timer_init(&flash_timer, flash_timer_expiry, NULL);

    usb_present = usb;
    state = boot_state;
    state_enter(boot_state);

    last_change_ms = k_uptime_get();

    /* Booting into an Active Mode counts as inactive -> active */
    if (light_is_on(boot_state)) {
        selection_ticks = 0;
    }

    atomic_set(&published_state, boot_state);
    status_refresh();

    debug_printk("State machine started in state %d (USB %s)\n",
                 boot_state, usb ? "connected" : "disconnected");
}

void sm_run(void)
{
    struct app_event evt;

    while (1) {
        if (app_event_get(&evt, K_FOREVER) == 0) {
            handle_event(&evt);
        }
    }
}

enum system_state sm_first_active_mode(void)
{
    return mode_cycle[0];
}

uint8_t sm_current_state(void)
{
    return (uint8_t)atomic_get(&published_state);
}
