/*
 * ANT+ rear light firmware (SPEC.md).
 *
 * Structure follows the Zephyr firmware it replaces: interrupts and timers
 * only post events, and this loop is the single consumer. Events of the
 * state machine go to sm.c; driver events (SoftDevice, button, USB poll,
 * sensor ticks) are handled here and may post state machine events.
 */

#include "ant_link.h"
#include "app_events.h"
#include "battery.h"
#include "battery_logic.h"
#include "bike_light.h"
#include "button.h"
#include "config.h"
#include "light.h"
#include "power.h"
#include "sd_includes.h"
#include "sensors.h"
#include "sm.h"
#include "soft_timer.h"
#include "stats.h"
#include "status_led.h"

#define LFCLK_START_WAIT_LIMIT 2000000U

/* The ANT+ modes use the numbering of the lamp modes of bike_light.c */
_Static_assert((int)LAMP_OFF == (int)LED_OFF && (int)LAMP_STEADY == (int)LED_STEADY &&
               (int)LAMP_FLASH == (int)LED_FLASH && (int)LAMP_SMART == (int)LED_SMART,
               "lamp modes and system states must share their numbering");

static enum lamp_mode lamp_mode_of_state(enum system_state s)
{
    /* Deep Sleep is published as Off, Idle Charging is reported as Off, both
     * Smart variants as Smart (ANT-3) */
    if (s == LED_SMART_AUTO_OFF) {
        return LAMP_SMART;
    }
    return (s <= LED_SMART) ? (enum lamp_mode)s : LAMP_OFF;
}

/* --- Operations for the state machine ----------------------------------- */

static void op_flash_timer_start(uint32_t ms)
{
    soft_timer_start(ST_FLASH, ms, 0U);
}

static void op_flash_timer_stop(void)
{
    soft_timer_stop(ST_FLASH);
}

static enum battery_level op_battery_sample(void)
{
    enum battery_level level = battery_sample();

    proto_stats.battery_mv = battery_last_mv();
    return level;
}

static void op_sleep_enter(void)
{
    ant_link_close();
}

static void op_sleep_exit(void)
{
    ant_link_open();
}

static void op_state_changed(enum system_state s)
{
    ant_link_report_mode(lamp_mode_of_state(s));
    proto_stats.mode_changes++;
}

static uint32_t op_now_ms(void)
{
    return soft_timer_uptime_ms();
}

static void op_delay_ms(uint32_t ms)
{
    uint32_t start = soft_timer_uptime_ms();

    while ((uint32_t)(soft_timer_uptime_ms() - start) < ms) {
    }
}

static void op_reset(void)
{
    NVIC_SystemReset();
}

static const struct sm_ops ops = {
    .light_set = light_set,
    .status_set = status_led_set,
    .status_off = status_led_off,
    .flash_timer_start = op_flash_timer_start,
    .flash_timer_stop = op_flash_timer_stop,
    .sensors_start = sensors_start,
    .sensors_start_parked = sensors_start_parked,
    .sensors_stop = sensors_stop,
    .battery_sample = op_battery_sample,
    .battery_tick_start = battery_tick_start,
    .battery_tick_stop = battery_tick_stop,
    .button_is_pressed = button_is_pressed,
    .button_arm_wake = button_arm_wake,
    .button_reenable = button_reenable,
    .sleep_enter = op_sleep_enter,
    .sleep_exit = op_sleep_exit,
    .state_changed = op_state_changed,
    .now_ms = op_now_ms,
    .delay_ms = op_delay_ms,
    .reset = op_reset,
};

/* --- ANT --------------------------------------------------------------- */

/* A controller request becomes an event like any other input (SYS-3) */
static void on_remote_mode_request(enum lamp_mode mode)
{
    (void)app_event_post(EVT_REMOTE_SET_STATE, (uint8_t)mode);
}

static void report_battery(void)
{
    uint32_t mv = battery_last_mv();

    ant_link_report_battery(battery_logic_ant_status(mv, sm_usb_present()),
                            battery_logic_percent(mv));
}

/* Without the SoftDevice nothing starts the low-frequency clock that RTC1
 * (button sampling, flash, blink) runs from */
static void start_lfclk_without_softdevice(void)
{
    uint32_t guard = LFCLK_START_WAIT_LIMIT;

    NRF_CLOCK->LFCLKSRC = CLOCK_LFCLKSRC_SRC_RC << CLOCK_LFCLKSRC_SRC_Pos;
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0U;
    NRF_CLOCK->TASKS_LFCLKSTART = 1U;
    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0U && guard-- > 0U) {
    }
}

/* --- Event loop -------------------------------------------------------- */

static void handle_event(const struct app_event *evt)
{
    switch (evt->type) {
    case EVT_SD:
        ant_link_process();
        break;
    case EVT_BUTTON_EDGE:
        button_on_edge();
        break;
    case EVT_BUTTON_SAMPLE:
        button_on_sample();
        break;
    case EVT_STATUS_BLINK:
        status_led_on_blink();
        break;
    case EVT_USB_POLL:
        power_usb_poll();
        break;
    case EVT_SENSOR_TICK:
        sensors_on_tick();
        break;
    case EVT_BLANK_STEP:
        sensors_on_blank_step();
        break;
    case EVT_BATTERY_TICK:
        sm_handle_event(evt);
        ant_link_on_battery_tick();
        report_battery();
        break;
    case EVT_USB_CONNECTED:
    case EVT_USB_DISCONNECTED:
        sm_handle_event(evt);
        report_battery(); /* charging is part of the battery status */
        break;
    default:
        sm_handle_event(evt);
        break;
    }
}

#ifdef BENCH_BUILD
/* Post an event written into the debug counters over SWD (bench builds only):
 * lets the bench exercise button, USB and controller paths without hardware */
static void poll_inject(void)
{
    uint32_t request = proto_stats.inject;

    if ((request & (1UL << 16)) != 0U) {
        proto_stats.inject = 0U;
        (void)app_event_post((uint8_t)(request & 0xFFU), (uint8_t)((request >> 8) & 0xFFU));
    }
}
#endif

static void publish_stats(void)
{
    proto_stats.uptime_s = soft_timer_uptime_ms() / 1000U;
    proto_stats.state = (uint8_t)sm_current_state();
    proto_stats.sleeping = sm_is_sleeping() ? 1U : 0U;
    proto_stats.parked = sm_is_parked() ? 1U : 0U;
    proto_stats.usb = sm_usb_present() ? 1U : 0U;
    proto_stats.low_battery = sm_low_battery() ? 1U : 0U;
    proto_stats.events_dropped = app_event_dropped();
    proto_stats.mode = (uint8_t)lamp_mode_of_state(sm_current_state());
}

int main(void)
{
    uint32_t reset_reason = power_take_reset_reason();
    bool usb;
    struct app_event evt;

    proto_stats.reset_reason = reset_reason;

    light_init();
    status_led_init();

    /* Non-fatal: without the SoftDevice the light works locally (SYS-5) */
    if (ant_link_init(on_remote_mode_request) != 0U) {
        start_lfclk_without_softdevice();
    }

    /* The main loop sleeps inside a critical region, which masks the
     * application interrupts in the NVIC. Without SEVONPEND a masked
     * interrupt that becomes pending does not end sd_app_evt_wait() or WFE,
     * and timer events would wait for the next SoftDevice event (~125 ms). */
    SCB->SCR |= SCB_SCR_SEVONPEND_Msk;
    soft_timer_init();

    /* Non-fatal: Smart works with the remaining sensor, or stays dark */
    (void)sensors_init();

    if (button_init() != 0) {
        NVIC_SystemReset(); /* essential initialization failed (SYS-5, ADR-0003) */
    }

    usb = power_usb_present();
    (void)battery_read_mv(); /* first value for the ANT+ pages */

    app_event_reset();
    sm_init(&ops, sm_boot_state(power_reset_is_power_on(reset_reason), usb), usb);
    battery_tick_start();
    power_usb_poll_start();
    ant_link_open();
    report_battery();

    /* Arm the button last, after the slow initialization, so no spurious
     * event is queued during boot */
    if (button_enable() != 0) {
        NVIC_SystemReset();
    }

    for (;;) {
#ifdef BENCH_BUILD
        poll_inject();
#endif
        if (app_event_get(&evt)) {
            handle_event(&evt);
            publish_stats();
        } else {
            ant_link_sleep_until_event();
        }
    }
}
