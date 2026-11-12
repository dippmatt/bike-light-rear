/*
 * Find My advertiser (ADR-0011): sends the advert of haystack_adv.h as a raw
 * BLE packet on channels 37/38/39 inside a Radio Timeslot of the S212
 * SoftDevice, next to the ANT channel. No BLE stack is involved.
 *
 * Runs in every awake state including Deep Sleep; nothing in the state
 * machine depends on it. Compiled only when the build has a key (HAYSTACK).
 */

#ifndef HAYSTACK_H
#define HAYSTACK_H

#include <stdbool.h>
#include <stdint.h>

/* Debug counters, readable over SWD (arm-zephyr-eabi-nm for the address) */
struct haystack_stats {
    uint32_t adverts;   /* complete adverts (3 channels) sent */
    uint32_t blocked;   /* timeslot requests the SoftDevice could not place */
    uint32_t canceled;  /* timeslots the SoftDevice took away */
    uint32_t overruns;  /* timeslots ended by the safety timer */
    uint32_t invalid;   /* signal callback returns the SoftDevice rejected */
    uint32_t last_error;/* last non-zero return code of a radio API call */
    uint32_t starts;    /* timeslots that started */
    /* AIRTAG_ROLLING only */
    uint32_t index;       /* Apple key index being sent */
    uint32_t derive_ms;   /* time of the last key derivation (0 at boot: no clock yet) */
    uint32_t flash_errors;/* key store writes or erases the SoftDevice could not do */
};

extern volatile struct haystack_stats haystack_stats;

/* After sd_softdevice_enable. Returns an NRF_ error code. */
uint32_t haystack_init(void);

/* Call from the main loop every HAYSTACK_POLL_MS: rotates the key when due
 * and retries key store writes (AIRTAG_ROLLING; no-op otherwise). The key
 * derivation takes tens of milliseconds, so it may not run in the timeslot
 * callback. */
void haystack_poll(void);

/* Feed every SoC event from sd_evt_get(); handles the timeslot and flash ones */
void haystack_on_soc_event(uint32_t evt);

#endif /* HAYSTACK_H */
