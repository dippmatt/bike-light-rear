/*
 * Rear light control from the front light, see rear_link.h.
 */

#include "rear_link.h"

#include "config.h"

void rear_link_init(struct rear_link *rl)
{
    rl->phase = REAR_SEARCHING;
    rl->device_number = 0U;
    rl->pending = false;
    rl->newer = false;
    rl->in_flight = false;
    rl->verifying = false;
    rl->mismatches = 0U;
    rl->off = false;
    rl->ant_mode = 0U;
    rl->retries = 0U;
    rl->seq = 0U;
}

void rear_link_forget(struct rear_link *rl)
{
    rl->phase = REAR_SEARCHING;
    rl->device_number = 0U;
    rl->in_flight = false;
    rl->newer = false;
    rl->retries = 0U;
    if (rl->verifying) {
        rl->verifying = false; /* not confirmed: send it again to the next rear */
    }
}

void rear_link_drop(struct rear_link *rl)
{
    rl->pending = false;
    rl->newer = false;
    rl->in_flight = false;
    rl->verifying = false;
    rl->retries = 0U;
}

/* Front state to rear command, the single configuration point of the
 * mapping (REAR-3): Smart -> Smart, Standard -> Steady, Bright -> Flash,
 * everything that is not an Active Mode -> Off */
static void map_state(enum system_state state, bool *off, uint8_t *ant_mode)
{
    *off = false;
    switch (state) {
    case LED_SMART:
    case LED_SMART_AUTO_OFF:
        *ant_mode = REAR_ANT_MODE_AUTO;
        break;
    case LED_STEADY:
        *ant_mode = REAR_ANT_MODE_STEADY;
        break;
    case LED_BRIGHT:
    case LED_FLASH:
        *ant_mode = REAR_ANT_MODE_FLASH;
        break;
    default:
        *off = true;
        *ant_mode = 0U;
        break;
    }
}

static enum rear_link_action send_if_ready(const struct rear_link *rl)
{
    return (rl->phase == REAR_FOUND && rl->pending && !rl->in_flight && !rl->verifying)
               ? REAR_ACTION_SEND
               : REAR_ACTION_NONE;
}

/* A failed attempt (transfer failed, or acked but not applied): try again
 * within the retry budget, then give up until the next local change */
static enum rear_link_action retry(struct rear_link *rl)
{
    rl->retries++;
    if (rl->retries > REAR_CMD_RETRIES) {
        rl->pending = false;
        return REAR_ACTION_NONE;
    }
    return send_if_ready(rl);
}

/* An acknowledgement only says the rear received the command; it ignores
 * one that arrives within its stable window after its own button press
 * (ANT-8). The mode it reports in page 1 shows whether it was applied. */
static enum rear_link_action verify(struct rear_link *rl, const uint8_t page[BL_PAGE_SIZE])
{
    uint8_t reported = (uint8_t)(page[6] >> 2);
    uint8_t expected = rl->off ? 0U : rl->ant_mode;

    if (page[0] != BL_PAGE_LIGHT_STATE) {
        return REAR_ACTION_NONE;
    }
    if (reported == expected) {
        rl->verifying = false;
        rl->pending = false;
        return REAR_ACTION_NONE;
    }
    rl->mismatches++;
    if (rl->mismatches < REAR_VERIFY_PAGES) {
        return REAR_ACTION_NONE; /* the rear may not have reported it yet */
    }
    rl->verifying = false;
    return retry(rl);
}

enum rear_link_action rear_link_request(struct rear_link *rl, enum system_state state)
{
    map_state(state, &rl->off, &rl->ant_mode);
    rl->pending = true;
    rl->verifying = false; /* a new command replaces the one being checked */
    rl->retries = 0U;
    rl->seq++;
    if (rl->in_flight) {
        rl->newer = true; /* sent when the transfer in flight ends */
        return REAR_ACTION_NONE;
    }
    return send_if_ready(rl);
}

enum rear_link_action rear_link_on_page(struct rear_link *rl, uint16_t device_number,
                                        const uint8_t page[BL_PAGE_SIZE])
{
    if (rl->phase == REAR_SEARCHING) {
        rl->device_number = device_number;
        rl->phase = REAR_CHECKING;
    }

    if (rl->phase == REAR_CHECKING) {
        if (page[0] != BL_PAGE_LIGHT_STATE) {
            return REAR_ACTION_NONE; /* the light type comes with page 1 */
        }
        if (((page[2] >> 2) & 0x07U) != REAR_LIGHT_TYPE) {
            rl->phase = REAR_SEARCHING; /* e.g. another rider's front light */
            return REAR_ACTION_EXCLUDE;
        }
        rl->phase = REAR_FOUND;
    }

    if (rl->verifying) {
        return verify(rl, page);
    }
    return send_if_ready(rl);
}

enum rear_link_action rear_link_on_tx_done(struct rear_link *rl, bool acked)
{
    rl->in_flight = false;

    if (rl->newer) {
        rl->newer = false; /* the newest command replaces the one in flight */
        return send_if_ready(rl);
    }
    if (acked) {
        rl->verifying = true; /* done once page 1 reports the mode */
        rl->mismatches = 0U;
        return REAR_ACTION_NONE;
    }
    return retry(rl);
}

void rear_link_on_closed(struct rear_link *rl)
{
    rl->in_flight = false; /* the transfer is lost; the command stays pending */
    rl->newer = false;
    if (rl->phase == REAR_CHECKING) {
        rl->phase = REAR_SEARCHING;
        rl->device_number = 0U;
    }
}

void rear_link_build(struct rear_link *rl, uint8_t page[BL_PAGE_SIZE])
{
    bike_light_build_settings(page, REAR_LIGHT_TYPE, rl->seq, rl->ant_mode, rl->off);
    rl->in_flight = true;
}

bool rear_link_busy(const struct rear_link *rl)
{
    return rl->pending && !rl->verifying && rl->phase == REAR_FOUND;
}
