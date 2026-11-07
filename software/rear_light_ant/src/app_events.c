/*
 * Central event queue, see app_events.h.
 */

#include "app_events.h"

#include "config.h"
#include "platform.h"

static struct app_event queue[APP_EVENT_QUEUE_DEPTH];
static uint8_t head;  /* next to read */
static uint8_t count; /* events waiting */
static uint32_t dropped;
static uint32_t coalesced_waiting; /* bit per driver event type waiting */

static uint32_t coalesce_bit(uint8_t type)
{
    return (type >= EVT_FIRST_DRIVER_EVENT) ? (1UL << (type - EVT_FIRST_DRIVER_EVENT)) : 0UL;
}

bool app_event_post(uint8_t type, uint8_t arg)
{
    uint8_t key = platform_lock();
    uint32_t bit = coalesce_bit(type);
    bool queued = false;

    if (bit != 0UL && (coalesced_waiting & bit) != 0UL) {
        queued = true; /* already waiting: this one is merged into it */
    } else if (count < APP_EVENT_QUEUE_DEPTH) {
        uint8_t tail = (uint8_t)((head + count) % APP_EVENT_QUEUE_DEPTH);

        queue[tail].type = type;
        queue[tail].arg = arg;
        count++;
        coalesced_waiting |= bit;
        queued = true;
    } else {
        dropped++;
    }

    platform_unlock(key);
    return queued;
}

bool app_event_get(struct app_event *evt)
{
    uint8_t key = platform_lock();
    bool got = false;

    if (count > 0U) {
        *evt = queue[head];
        head = (uint8_t)((head + 1U) % APP_EVENT_QUEUE_DEPTH);
        count--;
        coalesced_waiting &= ~coalesce_bit(evt->type);
        got = true;
    }

    platform_unlock(key);
    return got;
}

bool app_event_pending(void)
{
    return count > 0U;
}

uint32_t app_event_dropped(void)
{
    return dropped;
}

void app_event_reset(void)
{
    uint8_t key = platform_lock();

    head = 0U;
    count = 0U;
    dropped = 0U;
    coalesced_waiting = 0UL;

    platform_unlock(key);
}
