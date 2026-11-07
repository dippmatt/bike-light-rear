/*
 * ANT SoftDevice glue: enables the S212 SoftDevice, runs the bike light
 * master channel and moves 8-byte pages between the channel and the profile
 * logic in bike_light.c (SPEC section 10).
 *
 * If the SoftDevice cannot be enabled, ant_link_available() is false and
 * all other calls do nothing: the light works locally (SYS-5).
 */

#ifndef ANT_LINK_H
#define ANT_LINK_H

#include <stdbool.h>
#include <stdint.h>

#include "bike_light.h"
#include "sm.h"

enum ant_channel_state {
    ANT_CH_CLOSED = 0,
    ANT_CH_OPEN,
    ANT_CH_CLOSING,
};

typedef void (*ant_mode_request_cb)(enum lamp_mode mode);

/**
 * Enable the SoftDevice (low-frequency clock from the build: RC by default),
 * configure the channel. The channel is not opened yet.
 *
 * @return 0 on success, otherwise the failing SoftDevice return code
 *         (also stored in proto_stats)
 */
uint32_t ant_link_init(ant_mode_request_cb on_mode_request);

bool ant_link_available(void);

/** Open the channel (awake); a channel that is still closing opens as soon as it has closed */
void ant_link_open(void);

/** Close the channel (Deep Sleep, ANT-2) */
void ant_link_close(void);

/** Drain pending SoftDevice events (EVT_SD) */
void ant_link_process(void);

/** Report the state actually applied; restarts the page pattern on a change */
void ant_link_report_mode(enum lamp_mode mode);

void ant_link_report_battery(uint8_t status, uint8_t percent);

/**
 * Once per battery tick while awake: retries opening a failed channel and
 * counts controller silence (ANT-1, ANT-11).
 */
void ant_link_on_battery_tick(void);

/** Wait for an interrupt; returns when something needs service */
/** Front light: pass a change the rider made on to the rear (REAR-3); a
 *  no-op on the rear light */
void ant_link_rear_request(enum system_state state);

/** Front light: before Deep Sleep, wait up to @p timeout_ms for a pending
 *  command to be acknowledged, then drop it (REAR-6) */
void ant_link_rear_flush(uint32_t timeout_ms);

void ant_link_sleep_until_event(void);

#endif /* ANT_LINK_H */
