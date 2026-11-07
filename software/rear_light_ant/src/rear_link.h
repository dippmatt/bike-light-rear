/*
 * Front light only: control of the rear light as an ANT+ Bike Lights
 * controller (SPEC section 12, ADR-0010). Portable decision logic, no
 * hardware: ant_link.c owns the second ANT channel and asks this module
 * what to do. One command is pending at a time; the newest local change
 * wins.
 */

#ifndef REAR_LINK_H
#define REAR_LINK_H

#include <stdbool.h>
#include <stdint.h>

#include "bike_light.h"
#include "sm.h"

/* The rear's ANT+ mode numbers (its mode map, SPEC ANT-3 rear) */
#define REAR_ANT_MODE_STEADY 5U
#define REAR_ANT_MODE_FLASH  6U
#define REAR_ANT_MODE_AUTO   9U
#define REAR_LIGHT_TYPE      2U /* taillight */

enum rear_link_phase {
    REAR_SEARCHING = 0, /* no rear yet */
    REAR_CHECKING,      /* a device was acquired, its light type is not yet known */
    REAR_FOUND,         /* a taillight: commands go to it */
};

/* What the driver has to do after a call */
enum rear_link_action {
    REAR_ACTION_NONE = 0,
    REAR_ACTION_SEND,    /* build the pending command and send it (acknowledged) */
    REAR_ACTION_EXCLUDE, /* the acquired device is no taillight: exclude it and search again */
};

struct rear_link {
    enum rear_link_phase phase;
    uint16_t device_number; /* 0 until acquired */
    bool pending;           /* a command waits to be delivered */
    bool newer;             /* a newer command arrived while one was in flight */
    bool in_flight;
    bool verifying;         /* acked: waiting for page 1 to report the mode (REAR-5) */
    uint8_t mismatches;     /* page 1s with another mode since the ack */
    bool off;               /* pending command: beam off */
    uint8_t ant_mode;       /* pending command: rear ANT+ mode */
    uint8_t retries;
    uint8_t seq;
};

void rear_link_init(struct rear_link *rl);

/** Forget the rear (front wakes, or reset): the next one is searched anew. A
 *  pending command is kept, it goes to the rear found next (REAR-2). */
void rear_link_forget(struct rear_link *rl);

/** Drop a pending command (front goes to sleep after the flush) */
void rear_link_drop(struct rear_link *rl);

/** The rider changed the front light to @p state (REAR-3, REAR-5) */
enum rear_link_action rear_link_request(struct rear_link *rl, enum system_state state);

/** A page was received from @p device_number on the rear channel */
enum rear_link_action rear_link_on_page(struct rear_link *rl, uint16_t device_number,
                                        const uint8_t page[BL_PAGE_SIZE]);

/** The acknowledged transfer of the command in flight ended */
enum rear_link_action rear_link_on_tx_done(struct rear_link *rl, bool acked);

/** The rear channel closed (search timeout, exclusion, sleep) */
void rear_link_on_closed(struct rear_link *rl);

/** Build the pending command into @p page and mark it in flight */
void rear_link_build(struct rear_link *rl, uint8_t page[BL_PAGE_SIZE]);

/** True while a command waits to be acknowledged by a found rear (flush
 *  before sleep; the check of the reported mode is not waited for) */
bool rear_link_busy(const struct rear_link *rl);

#endif /* REAR_LINK_H */
