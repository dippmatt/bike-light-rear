/*
 * Software timers on RTC1, see soft_timer.h.
 */

#include "soft_timer.h"

#include "app_events.h"
#include "sd_includes.h"

#define RTC_TICK_HZ 1024U /* 32768 Hz / (PRESCALER + 1) with PRESCALER = 31 */
#define RTC_MASK 0x00FFFFFFUL
#define RTC_HALF 0x00800000UL
#define RTC_MIN_COMPARE_TICKS 2U /* RTC cannot compare closer than 2 ticks */
#define RTC_IRQ_PRIORITY 6U

struct timer {
    bool active;
    uint32_t due;    /* RTC tick at which it fires */
    uint32_t period; /* ticks, 0 = one-shot */
};

static struct timer timers[ST_COUNT];

static const uint8_t timer_event[ST_COUNT] = {
    [ST_FLASH] = EVT_FLASH_STEP,
    [ST_BUTTON] = EVT_BUTTON_SAMPLE,
    [ST_BATTERY] = EVT_BATTERY_TICK,
    [ST_STATUS] = EVT_STATUS_BLINK,
    [ST_USB] = EVT_USB_POLL,
    [ST_SENSOR] = EVT_SENSOR_TICK,
    [ST_BLANK] = EVT_BLANK_STEP,
};

/* The 24-bit counter wraps every 2^24 / 1024 s (4.5 h): count the wraps */
static volatile uint32_t overflows;

static uint32_t ms_to_ticks(uint32_t ms)
{
    uint32_t ticks = (ms * RTC_TICK_HZ + 999U) / 1000U;

    return (ticks < RTC_MIN_COMPARE_TICKS) ? RTC_MIN_COMPARE_TICKS : ticks;
}

/* Program the compare register for the earliest active timer */
static void schedule(void)
{
    uint32_t now = NRF_RTC1->COUNTER;
    uint32_t best = RTC_HALF;
    bool any = false;
    uint32_t target;
    uint32_t left;

    for (unsigned int i = 0; i < ST_COUNT; i++) {
        uint32_t remaining;

        if (!timers[i].active) {
            continue;
        }
        remaining = (timers[i].due - now) & RTC_MASK;
        if (remaining >= RTC_HALF) {
            remaining = 0U; /* already due */
        }
        if (remaining < best) {
            best = remaining;
        }
        any = true;
    }

    if (!any) {
        NRF_RTC1->INTENCLR = RTC_INTENCLR_COMPARE0_Msk;
        return;
    }

    if (best < RTC_MIN_COMPARE_TICKS) {
        best = RTC_MIN_COMPARE_TICKS;
    }
    target = (now + best) & RTC_MASK;
    NRF_RTC1->CC[0] = target;
    NRF_RTC1->EVENTS_COMPARE[0] = 0U;
    NRF_RTC1->INTENSET = RTC_INTENSET_COMPARE0_Msk;

    /* If time moved past the target while programming, the compare event
     * would be missed: trigger the handler by hand */
    left = (target - NRF_RTC1->COUNTER) & RTC_MASK;
    if (left < RTC_MIN_COMPARE_TICKS || left >= RTC_HALF) {
        (void)sd_nvic_SetPendingIRQ(RTC1_IRQn);
    }
}

void RTC1_IRQHandler(void)
{
    uint32_t now;

    if (NRF_RTC1->EVENTS_OVRFLW != 0U) {
        NRF_RTC1->EVENTS_OVRFLW = 0U;
        overflows++;
    }
    NRF_RTC1->EVENTS_COMPARE[0] = 0U;
    (void)NRF_RTC1->EVENTS_COMPARE[0]; /* make sure the clear has reached the peripheral */

    now = NRF_RTC1->COUNTER;
    for (unsigned int i = 0; i < ST_COUNT; i++) {
        if (!timers[i].active) {
            continue;
        }
        if (((now - timers[i].due) & RTC_MASK) >= RTC_HALF) {
            continue; /* not due yet */
        }

        (void)app_event_post(timer_event[i], 0U);
        if (timers[i].period == 0U) {
            timers[i].active = false;
        } else {
            do {
                timers[i].due = (timers[i].due + timers[i].period) & RTC_MASK;
            } while (((now - timers[i].due) & RTC_MASK) < RTC_HALF);
        }
    }

    schedule();
}

void soft_timer_init(void)
{
    NRF_RTC1->PRESCALER = 31U;
    NRF_RTC1->EVTENSET = RTC_EVTENSET_COMPARE0_Msk | RTC_EVTENSET_OVRFLW_Msk;
    NRF_RTC1->INTENSET = RTC_INTENSET_OVRFLW_Msk;
    NRF_RTC1->TASKS_CLEAR = 1U;
    NRF_RTC1->TASKS_START = 1U;

    (void)sd_nvic_SetPriority(RTC1_IRQn, RTC_IRQ_PRIORITY);
    (void)sd_nvic_EnableIRQ(RTC1_IRQn);
}

void soft_timer_start(enum soft_timer_id id, uint32_t delay_ms, uint32_t period_ms)
{
    uint8_t nested;

    (void)sd_nvic_critical_region_enter(&nested);

    timers[id].due = (NRF_RTC1->COUNTER + ms_to_ticks(delay_ms)) & RTC_MASK;
    timers[id].period = (period_ms == 0U) ? 0U : ms_to_ticks(period_ms);
    timers[id].active = true;
    schedule();

    (void)sd_nvic_critical_region_exit(nested);
}

void soft_timer_stop(enum soft_timer_id id)
{
    uint8_t nested;

    (void)sd_nvic_critical_region_enter(&nested);

    timers[id].active = false;
    schedule();

    (void)sd_nvic_critical_region_exit(nested);
}

uint32_t soft_timer_uptime_ms(void)
{
    uint8_t nested;
    uint32_t wraps;
    uint32_t counter;
    uint64_t ticks;

    (void)sd_nvic_critical_region_enter(&nested);

    wraps = overflows;
    counter = NRF_RTC1->COUNTER;
    /* The wrap may have happened while interrupts were masked: its event is
     * set but the handler has not counted it yet */
    if (NRF_RTC1->EVENTS_OVRFLW != 0U && counter < RTC_HALF) {
        wraps++;
    }

    (void)sd_nvic_critical_region_exit(nested);

    ticks = ((uint64_t)wraps << 24) + counter;
    return (uint32_t)((ticks * 125U) >> 7); /* ticks * 1000 / 1024 */
}
