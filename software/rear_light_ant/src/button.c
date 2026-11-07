/*
 * Button, see button.h.
 */

#include "button.h"

#include "app_events.h"
#include "board.h"
#include "config.h"
#include "sd_includes.h"
#include "soft_timer.h"

#define GPIOTE_IRQ_PRIORITY 6U
#define RELEASE_STABLE_POLLS 2U /* polls that must read released before re-arming */

enum button_state {
    BTN_DISARMED,     /* nothing armed */
    BTN_IDLE,         /* sense interrupt armed, waiting for a press */
    BTN_WAKE,         /* sense interrupt armed for Deep Sleep wake */
    BTN_SAMPLING,     /* press accepted, measuring its length */
    BTN_RELEASE_WAIT, /* press reported or ignored, waiting for a stable release */
};

static volatile enum button_state state = BTN_DISARMED;
static uint8_t press_counter;
static uint8_t released_polls;
static uint32_t last_press_ms;
static bool have_last_press;

bool button_is_pressed(void)
{
    return (NRF_P1->IN & (1UL << PIN_BUTTON)) == 0U;
}

static void set_sense(bool enabled)
{
    NRF_P1->PIN_CNF[PIN_BUTTON] =
        (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
        (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
        (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos) |
        ((enabled ? GPIO_PIN_CNF_SENSE_Low : GPIO_PIN_CNF_SENSE_Disabled)
         << GPIO_PIN_CNF_SENSE_Pos);
}

/* Arm the level-sensitive press detection for the given state */
static void arm(enum button_state armed_state)
{
    set_sense(true);
    NRF_P1->LATCH = 1UL << PIN_BUTTON;
    NRF_GPIOTE->EVENTS_PORT = 0U;
    NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_PORT_Msk;
    state = armed_state;
}

static void disarm(void)
{
    NRF_GPIOTE->INTENCLR = GPIOTE_INTENCLR_PORT_Msk;
    set_sense(false);
    soft_timer_stop(ST_BUTTON);
    state = BTN_DISARMED;
}

/* Sense is level sensitive: it fires again for as long as the button is
 * held, so the interrupt disarms the sense and only posts one event */
void GPIOTE_IRQHandler(void)
{
    if (NRF_GPIOTE->EVENTS_PORT != 0U) {
        NRF_GPIOTE->EVENTS_PORT = 0U;
        (void)NRF_GPIOTE->EVENTS_PORT;
        NRF_GPIOTE->INTENCLR = GPIOTE_INTENCLR_PORT_Msk;
        set_sense(false);
        (void)app_event_post(EVT_BUTTON_EDGE, 0U);
    }
}

int button_init(void)
{
    set_sense(false);
    (void)sd_nvic_SetPriority(GPIOTE_IRQn, GPIOTE_IRQ_PRIORITY);
    (void)sd_nvic_EnableIRQ(GPIOTE_IRQn);
    state = BTN_DISARMED;
    return 0;
}

int button_reenable(void)
{
    soft_timer_stop(ST_BUTTON);
    press_counter = 0U;

    if (button_is_pressed()) {
        /* Still held (a wake press, or held during boot): no event until it
         * has been released and pressed again (BOOT-4) */
        NRF_GPIOTE->INTENCLR = GPIOTE_INTENCLR_PORT_Msk;
        set_sense(false);
        released_polls = 0U;
        state = BTN_RELEASE_WAIT;
        soft_timer_start(ST_BUTTON, SAMPLE_INTERVAL_MS, SAMPLE_INTERVAL_MS);
    } else {
        arm(BTN_IDLE);
    }
    return 0;
}

int button_enable(void)
{
    return button_reenable();
}

int button_arm_wake(void)
{
    soft_timer_stop(ST_BUTTON);
    press_counter = 0U;
    arm(BTN_WAKE);
    return 0;
}

void button_on_edge(void)
{
    uint32_t now;

    switch (state) {
    case BTN_WAKE:
        disarm(); /* one-shot; button_reenable() arms the press detection again */
        (void)app_event_post(EVT_BUTTON_WAKE, 0U);
        return;
    case BTN_IDLE:
        break;
    default:
        return; /* stale edge */
    }

    if (!button_is_pressed()) {
        arm(BTN_IDLE); /* a glitch, not a press */
        return;
    }

    now = soft_timer_uptime_ms();
    if (have_last_press && (now - last_press_ms) < DEBOUNCE_MS) {
        /* Too close to the previous press (BTN-3): ignore it, wait for release */
        released_polls = 0U;
        state = BTN_RELEASE_WAIT;
        soft_timer_start(ST_BUTTON, SAMPLE_INTERVAL_MS, SAMPLE_INTERVAL_MS);
        return;
    }

    last_press_ms = now;
    have_last_press = true;
    press_counter = 0U;
    state = BTN_SAMPLING;
    soft_timer_start(ST_BUTTON, SAMPLE_INTERVAL_MS, SAMPLE_INTERVAL_MS);
}

void button_on_sample(void)
{
    switch (state) {
    case BTN_SAMPLING:
        if (!button_is_pressed()) {
            /* Released before the long-press threshold: short press */
            released_polls = 1U;
            state = BTN_RELEASE_WAIT;
            (void)app_event_post(EVT_BUTTON_SHORT, 0U);
            return;
        }
        press_counter++;
        if (press_counter >= LONG_PRESS_SAMPLES) {
            /* Held for 1 s: long press, reported while still held (BTN-2) */
            released_polls = 0U;
            state = BTN_RELEASE_WAIT;
            (void)app_event_post(EVT_BUTTON_LONG, 0U);
        }
        return;
    case BTN_RELEASE_WAIT:
        released_polls = button_is_pressed() ? 0U : (uint8_t)(released_polls + 1U);
        if (released_polls >= RELEASE_STABLE_POLLS) {
            soft_timer_stop(ST_BUTTON);
            arm(BTN_IDLE);
        }
        return;
    default:
        soft_timer_stop(ST_BUTTON); /* stale timer event */
        return;
    }
}
