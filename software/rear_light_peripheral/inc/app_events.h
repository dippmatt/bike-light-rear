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

#ifndef APP_EVENTS_H
#define APP_EVENTS_H

#include <stdint.h>
#include <zephyr/kernel.h>

/**
 * Central event bus of the application.
 *
 * Every input source (button, BLE, timers, sensors, battery, USB) posts
 * events into one message queue. The state machine thread is the single
 * consumer. Posters never read or modify system state directly.
 */

enum app_event_type {
    EVT_BUTTON_SHORT,       /* button released before long-press threshold */
    EVT_BUTTON_LONG,        /* button held for >= 1 s */
    EVT_BLE_SET_STATE,      /* arg = requested system_state (pre-validated 0..4) */
    EVT_FLASH_STEP,         /* flash-pattern timer tick (only used in FLASH mode) */
    EVT_BATTERY_TICK,       /* 5 s battery sampling tick */
    EVT_USB_CONNECTED,      /* VBUS appeared (edge) */
    EVT_USB_DISCONNECTED,   /* VBUS vanished (edge) */
    EVT_BRAKE_START,        /* braking detected (2-sample hysteresis) */
    EVT_BRAKE_STOP,         /* braking ended (2-sample hysteresis) */
    EVT_AMBIENT_DARK,       /* ambient light below dark threshold */
    EVT_AMBIENT_BRIGHT,     /* ambient light above bright threshold */
    EVT_STATIONARY_TIMEOUT, /* device stationary for the auto-off period */
};

struct app_event {
    uint8_t type; /* enum app_event_type */
    uint8_t arg;  /* event argument, only used by EVT_BLE_SET_STATE */
};

/**
 * @brief Post an event to the central event queue
 *
 * Safe to call from any context, including ISRs (never blocks).
 * If the queue is full the event is dropped with a warning; all event
 * sources are periodic or re-detecting, so a dropped event recurs.
 *
 * @param type Event type (enum app_event_type)
 * @param arg  Event argument (0 if unused)
 */
void app_event_post(uint8_t type, uint8_t arg);

/**
 * @brief Fetch the next event from the queue
 *
 * Must only be called by the state machine thread.
 *
 * @param evt     Destination for the received event
 * @param timeout Wait timeout (e.g. K_FOREVER)
 * @return 0 on success, negative error code otherwise
 */
int app_event_get(struct app_event *evt, k_timeout_t timeout);

#endif /* APP_EVENTS_H */
