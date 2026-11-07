/*
 * Host tests of the portable core: event queue, battery logic, detectors and
 * the state machine. Test names carry the SPEC requirement IDs they cover.
 * Build and run with: make test
 */

#include <stdio.h>
#include <string.h>

#include "../src/app_events.h"
#include "../src/battery_logic.h"
#include "../src/bike_light.h"
#include "../src/config.h"
#include "../src/detectors.h"
#include "../src/sm.h"

static int failures;
static const char *current_test;

#define CHECK(cond)                                                                    \
    do {                                                                               \
        if (!(cond)) {                                                                 \
            printf("FAIL %s (%s:%d): %s\n", current_test, __FILE__, __LINE__, #cond);  \
            failures++;                                                                \
        }                                                                              \
    } while (0)

/* --- Fake hardware ----------------------------------------------------- */

static struct {
    uint32_t now;
    uint32_t light;
    unsigned int light_calls;
    bool status_low, status_charging, status_on, status_reachable_off;
    unsigned int status_calls;
    unsigned int status_off_calls;
    bool flash_running;
    uint32_t flash_ms;
    unsigned int flash_starts;
    bool sensors_running;
    unsigned int sensors_starts;
    enum battery_level battery;
    unsigned int battery_samples;
    bool battery_tick_running;
    bool pressed;
    unsigned int press_polls_until_release; /* pressed reads true this many times */
    int arm_wake_result;
    int reenable_result;
    unsigned int arm_wake_calls;
    unsigned int reenable_calls;
    unsigned int sleep_enter_calls;
    unsigned int sleep_exit_calls;
    enum system_state published;
    unsigned int published_calls;
    uint32_t delayed_ms;
    unsigned int resets;
} fake;

static void f_light_set(uint32_t p)
{
    fake.light = p;
    fake.light_calls++;
}

static void f_status_set(bool low, bool charging, bool on, bool reachable_off)
{
    fake.status_low = low;
    fake.status_charging = charging;
    fake.status_on = on;
    fake.status_reachable_off = reachable_off;
    fake.status_calls++;
}

static void f_status_off(void)
{
    fake.status_off_calls++;
    fake.status_low = fake.status_charging = fake.status_on = false;
    fake.status_reachable_off = false;
}

static void f_flash_start(uint32_t ms)
{
    fake.flash_running = true;
    fake.flash_ms = ms;
    fake.flash_starts++;
}

static void f_flash_stop(void)
{
    fake.flash_running = false;
}

static void f_sensors_start(void)
{
    fake.sensors_running = true;
    fake.sensors_starts++;
}

static void f_sensors_stop(void)
{
    fake.sensors_running = false;
}

static enum battery_level f_battery_sample(void)
{
    fake.battery_samples++;
    return fake.battery;
}

static void f_battery_tick_start(void)
{
    fake.battery_tick_running = true;
}

static void f_battery_tick_stop(void)
{
    fake.battery_tick_running = false;
}

static bool f_button_is_pressed(void)
{
    if (fake.press_polls_until_release > 0U) {
        fake.press_polls_until_release--;
        return true;
    }
    return fake.pressed;
}

static int f_arm_wake(void)
{
    fake.arm_wake_calls++;
    return fake.arm_wake_result;
}

static int f_reenable(void)
{
    fake.reenable_calls++;
    return fake.reenable_result;
}

static void f_sleep_enter(void)
{
    fake.sleep_enter_calls++;
}

static void f_sleep_exit(void)
{
    fake.sleep_exit_calls++;
}

static void f_state_changed(enum system_state s)
{
    fake.published = s;
    fake.published_calls++;
}

static uint32_t f_now(void)
{
    return fake.now;
}

static void f_delay(uint32_t ms)
{
    fake.now += ms;
    fake.delayed_ms += ms;
}

static void f_reset(void)
{
    fake.resets++;
}

static const struct sm_ops ops = {
    .light_set = f_light_set,
    .status_set = f_status_set,
    .status_off = f_status_off,
    .flash_timer_start = f_flash_start,
    .flash_timer_stop = f_flash_stop,
    .sensors_start = f_sensors_start,
    .sensors_stop = f_sensors_stop,
    .battery_sample = f_battery_sample,
    .battery_tick_start = f_battery_tick_start,
    .battery_tick_stop = f_battery_tick_stop,
    .button_is_pressed = f_button_is_pressed,
    .button_arm_wake = f_arm_wake,
    .button_reenable = f_reenable,
    .sleep_enter = f_sleep_enter,
    .sleep_exit = f_sleep_exit,
    .state_changed = f_state_changed,
    .now_ms = f_now,
    .delay_ms = f_delay,
    .reset = f_reset,
};

static void start(enum system_state boot, bool usb)
{
    memset(&fake, 0, sizeof(fake));
    fake.battery = BATTERY_OK;
    fake.battery_tick_running = true;
    app_event_reset();
    detectors_reset();
    battery_logic_reset();
    sm_init(&ops, boot, usb);
}

static void send(uint8_t type, uint8_t arg)
{
    struct app_event e = { type, arg };

    sm_handle_event(&e);
}

static void ticks(unsigned int n)
{
    for (unsigned int i = 0; i < n; i++) {
        send(EVT_BATTERY_TICK, 0U);
    }
}

/* Let the stable window (ANT-8) pass */
static void settle(void)
{
    fake.now += REMOTE_STABLE_WINDOW_MS + 1U;
}

#define TEST(name) \
    static void name(void)

#define BEGIN() (current_test = __func__)

/* --- Event queue ------------------------------------------------------- */

TEST(test_SYS3_SYS4_queue_order_and_drop)
{
    struct app_event e;

    BEGIN();
    app_event_reset();
    for (unsigned int i = 0; i < APP_EVENT_QUEUE_DEPTH; i++) {
        CHECK(app_event_post(EVT_REMOTE_SET_STATE, (uint8_t)i));
    }
    CHECK(!app_event_post(EVT_BUTTON_SHORT, 0U)); /* full: dropped and counted */
    CHECK(app_event_dropped() == 1U);

    for (unsigned int i = 0; i < APP_EVENT_QUEUE_DEPTH; i++) {
        CHECK(app_event_get(&e) && e.type == EVT_REMOTE_SET_STATE && e.arg == i);
    }
    CHECK(!app_event_get(&e));
    CHECK(!app_event_pending());
}

TEST(test_SYS4_driver_events_are_coalesced)
{
    struct app_event e;

    BEGIN();
    app_event_reset();
    for (unsigned int i = 0; i < 100U; i++) {
        CHECK(app_event_post(EVT_SD, 0U)); /* a burst of SoftDevice events */
    }
    CHECK(app_event_dropped() == 0U);
    CHECK(app_event_get(&e) && e.type == EVT_SD);
    CHECK(!app_event_get(&e));
    CHECK(app_event_post(EVT_SD, 0U)); /* possible again after it was taken */
    CHECK(app_event_get(&e));
}

/* --- Battery ------------------------------------------------------------- */

TEST(test_BAT2_BAT3_BAT4_battery_logic)
{
    BEGIN();
    battery_logic_reset();

    /* low: BATTERY_CONSECUTIVE samples below the low voltage */
    CHECK(battery_logic_process(BATTERY_LOW_MV - 1U) == BATTERY_OK);
    CHECK(battery_logic_process(BATTERY_LOW_MV - 1U) == BATTERY_OK);
    CHECK(battery_logic_process(BATTERY_LOW_MV - 1U) == BATTERY_LOW);
    /* leaves Low only after as many samples at or above the same voltage */
    CHECK(battery_logic_process(BATTERY_LOW_MV) == BATTERY_LOW);
    CHECK(battery_logic_process(BATTERY_LOW_MV) == BATTERY_LOW);
    CHECK(battery_logic_process(BATTERY_LOW_MV) == BATTERY_OK);

    /* critical needs consecutive samples; a good one resets the count */
    battery_logic_reset();
    CHECK(battery_logic_process(BATTERY_CRITICAL_MV - 1U) != BATTERY_CRITICAL);
    CHECK(battery_logic_process(BATTERY_CRITICAL_MV - 1U) != BATTERY_CRITICAL);
    CHECK(battery_logic_process(BATTERY_CRITICAL_MV) != BATTERY_CRITICAL);
    CHECK(battery_logic_process(BATTERY_CRITICAL_MV - 1U) != BATTERY_CRITICAL);
    CHECK(battery_logic_process(BATTERY_CRITICAL_MV - 1U) != BATTERY_CRITICAL);
    CHECK(battery_logic_process(BATTERY_CRITICAL_MV - 1U) == BATTERY_CRITICAL);

    /* a failed reading counts as 0 mV: an unmeasurable battery is empty */
    battery_logic_reset();
    battery_logic_process(0U);
    battery_logic_process(0U);
    CHECK(battery_logic_process(0U) == BATTERY_CRITICAL);
}

TEST(test_ANT12_battery_status_and_percent)
{
    BEGIN();
    CHECK(battery_logic_ant_status(4000U, true) == BL_BATT_CHARGING);
    CHECK(battery_logic_ant_status(0U, false) == BL_BATT_INVALID);
    CHECK(battery_logic_ant_status(0U, true) == BL_BATT_CHARGING);
    CHECK(battery_logic_ant_status(BATTERY_CRITICAL_MV - 1U, false) == BL_BATT_CRITICAL);
    CHECK(battery_logic_ant_status(BATTERY_LOW_MV - 1U, false) == BL_BATT_LOW);
    CHECK(battery_logic_ant_status(BATTERY_STATUS_OK_MV - 1U, false) == BL_BATT_OK);
    CHECK(battery_logic_ant_status(BATTERY_STATUS_GOOD_MV - 1U, false) == BL_BATT_GOOD);
    CHECK(battery_logic_ant_status(BATTERY_STATUS_GOOD_MV, false) == BL_BATT_NEW);

    CHECK(battery_logic_percent(0U) == 0xFFU);
    CHECK(battery_logic_percent(BATTERY_EMPTY_MV) == 0U);
    CHECK(battery_logic_percent(BATTERY_FULL_MV + 50U) == 100U);
    CHECK(battery_logic_percent((BATTERY_EMPTY_MV + BATTERY_FULL_MV) / 2U) == 50U);
}

/* --- Detectors ------------------------------------------------------------ */

static void feed_z(int64_t z_milli, unsigned int samples)
{
    for (unsigned int i = 0; i < samples; i++) {
        detectors_accel_sample(0, 0, z_milli);
    }
}

static unsigned int count_events(uint8_t type)
{
    struct app_event e;
    unsigned int n = 0U;

    while (app_event_get(&e)) {
        if (e.type == type) {
            n++;
        }
    }
    return n;
}

TEST(test_SMART11_sample_counts_round_up)
{
    BEGIN();
    CHECK(MS_TO_SAMPLES(100U, 50U) == 2U);
    CHECK(MS_TO_SAMPLES(101U, 50U) == 3U);
    CHECK(MS_TO_SAMPLES(1U, 50U) == 1U);
    CHECK(BRAKING_START_SAMPLES == 2U);
    CHECK(STATIONARY_SAMPLES == 3000U);
}

TEST(test_SMART3_SMART8_braking_start_stop_and_hold)
{
    BEGIN();
    app_event_reset();
    detectors_reset();

    /* the first sample seeds the baseline: a constant offset is not braking */
    feed_z(9810, 100);
    CHECK(!detectors_braking());
    CHECK(count_events(EVT_BRAKE_START) == 0U);

    /* a step of +6 m/s^2 above the baseline must hold for the confirm time */
    feed_z(9810 + 6000, 1);
    CHECK(!detectors_braking()); /* smoothing keeps the first sample below */
    feed_z(9810 + 6000, 8);
    CHECK(detectors_braking());
    CHECK(count_events(EVT_BRAKE_START) == 1U);

    /* below the threshold again (the smoothed value drops below it with the
     * first low sample): braking ends only after confirm + hold */
    feed_z(9810, BRAKING_STOP_SAMPLES - 1U);
    CHECK(detectors_braking());
    feed_z(9810, 1);
    CHECK(!detectors_braking());
    CHECK(count_events(EVT_BRAKE_STOP) == 1U);
}

TEST(test_SMART3_braking_again_within_hold_keeps_light_on)
{
    BEGIN();
    app_event_reset();
    detectors_reset();

    feed_z(0, 50);
    feed_z(7000, 10);
    CHECK(detectors_braking());
    (void)count_events(EVT_BRAKE_START);

    /* a gap shorter than the hold time, then braking again */
    feed_z(0, 6);
    feed_z(7000, 10);
    CHECK(detectors_braking());
    CHECK(count_events(EVT_BRAKE_STOP) == 0U);
}

TEST(test_SMART3_below_threshold_is_not_braking)
{
    BEGIN();
    app_event_reset();
    detectors_reset();

    feed_z(0, 50);
    feed_z(BRAKING_ACCEL_THRESHOLD_MILLI - 200, 100);
    CHECK(!detectors_braking());
}

TEST(test_SMART4_ambient_hysteresis)
{
    BEGIN();
    app_event_reset();
    detectors_reset();

    /* dark needs consecutive samples below the threshold */
    detectors_ambient_sample(AMBIENT_THRESHOLD_MILLILUX - 1);
    CHECK(!detectors_ambient_dark());
    detectors_ambient_sample(AMBIENT_THRESHOLD_MILLILUX); /* resets the count */
    detectors_ambient_sample(AMBIENT_THRESHOLD_MILLILUX - 1);
    CHECK(!detectors_ambient_dark());
    detectors_ambient_sample(AMBIENT_THRESHOLD_MILLILUX - 1);
    CHECK(detectors_ambient_dark());
    CHECK(count_events(EVT_AMBIENT_DARK) == 1U);

    /* bright again only at threshold + offset */
    detectors_ambient_sample(AMBIENT_OFF_THRESHOLD_MILLILUX - 1);
    detectors_ambient_sample(AMBIENT_OFF_THRESHOLD_MILLILUX - 1);
    CHECK(detectors_ambient_dark());
    detectors_ambient_sample(AMBIENT_OFF_THRESHOLD_MILLILUX);
    CHECK(detectors_ambient_dark());
    detectors_ambient_sample(AMBIENT_OFF_THRESHOLD_MILLILUX);
    CHECK(!detectors_ambient_dark());
    CHECK(count_events(EVT_AMBIENT_BRIGHT) == 1U);
}

TEST(test_SMART5_stationary_timeout_and_reset)
{
    BEGIN();
    app_event_reset();
    detectors_reset();

    /* 1 g on Z: stationary */
    for (unsigned int i = 0; i < STATIONARY_SAMPLES - 1U; i++) {
        detectors_accel_sample(0, 0, 9810);
    }
    CHECK(count_events(EVT_STATIONARY_TIMEOUT) == 0U);
    /* a sample outside the band resets the count */
    detectors_accel_sample(0, 0, 25000);
    detectors_accel_sample(0, 0, 9810);
    CHECK(count_events(EVT_STATIONARY_TIMEOUT) == 0U);

    for (unsigned int i = 0; i < STATIONARY_SAMPLES; i++) {
        detectors_accel_sample(1000, 2000, 9810);
    }
    CHECK(count_events(EVT_STATIONARY_TIMEOUT) == 1U);
}

TEST(test_SMART12_SMART13_light_plan)
{
    BEGIN();
    app_event_reset();
    detectors_reset();

    /* LED off (bright): read without blank */
    CHECK(detectors_light_plan() == LIGHT_PLAN_READ);
    CHECK(detectors_light_next_ticks(true) == LIGHT_TICKS);

    /* dark: blanked measurement at the long interval ... */
    detectors_ambient_sample(0);
    detectors_ambient_sample(0);
    CHECK(detectors_ambient_dark());
    CHECK(detectors_light_plan() == LIGHT_PLAN_BLANKED);
    CHECK(detectors_light_next_ticks(true) == LIGHT_BLANK_TICKS);

    /* ... but a reading that starts to end the darkness is confirmed sooner */
    detectors_ambient_sample(AMBIENT_OFF_THRESHOLD_MILLILUX);
    CHECK(detectors_light_next_ticks(true) == LIGHT_TICKS);
    CHECK(detectors_light_next_ticks(false) == LIGHT_TICKS);

    /* no measurement while braking */
    feed_z(0, 50);
    feed_z(9000, 10);
    CHECK(detectors_braking());
    CHECK(detectors_light_plan() == LIGHT_PLAN_SKIP);
}

/* --- State machine: boot, modes, button ----------------------------------- */

TEST(test_BOOT1_BOOT2_boot_policy)
{
    BEGIN();
    CHECK(sm_boot_state(true, true) == IDLE_CHARGING);
    CHECK(sm_boot_state(true, false) == sm_first_active_mode());
    CHECK(sm_boot_state(false, true) == sm_first_active_mode());
    CHECK(sm_boot_state(false, false) == sm_first_active_mode());
}

TEST(test_SYS2_mode_cycle_order)
{
    BEGIN();
    CHECK(sm_first_active_mode() == LED_SMART);
    start(LED_OFF, false);
    send(EVT_BUTTON_SHORT, 0U);
    CHECK(sm_current_state() == LED_SMART);
    send(EVT_BUTTON_SHORT, 0U);
    CHECK(sm_current_state() == LED_STEADY);
    send(EVT_BUTTON_SHORT, 0U);
    CHECK(sm_current_state() == LED_FLASH);
}

TEST(test_BTN4_short_press_from_off_and_idle_charging)
{
    BEGIN();
    start(LED_OFF, false);
    send(EVT_BUTTON_SHORT, 0U);
    CHECK(sm_current_state() == sm_first_active_mode());

    start(IDLE_CHARGING, true);
    send(EVT_BUTTON_SHORT, 0U);
    CHECK(sm_current_state() == sm_first_active_mode());
}

TEST(test_BTN4_end_of_cycle_goes_to_idle_charging_or_deep_sleep)
{
    BEGIN();
    start(LED_FLASH, true); /* the last Active Mode, USB present */
    send(EVT_BUTTON_SHORT, 0U);
    CHECK(sm_current_state() == IDLE_CHARGING);
    CHECK(!sm_is_sleeping());

    start(LED_FLASH, false);
    send(EVT_BUTTON_SHORT, 0U);
    CHECK(sm_is_sleeping());
    CHECK(fake.sleep_enter_calls == 1U);
}

TEST(test_BTN8_BTN9_selection_window)
{
    BEGIN();
    start(LED_OFF, false);
    send(EVT_BUTTON_SHORT, 0U); /* first Active Mode, window opens */
    ticks(MODE_SELECT_WINDOW_BATTERY_TICKS - 1U);
    send(EVT_BUTTON_SHORT, 0U);
    CHECK(sm_current_state() == LED_STEADY); /* still inside */

    /* the window closes after N ticks, no visible effect at that moment */
    ticks(1U);
    CHECK(sm_current_state() == LED_STEADY);
    CHECK(fake.light == LIGHT_PULSE_BASE);
    /* the next short press leaves the cycle instead of stepping on */
    send(EVT_BUTTON_SHORT, 0U);
    CHECK(sm_is_sleeping());
}

TEST(test_BTN8_window_closes_after_ticks_and_not_restarted_by_mode_change)
{
    BEGIN();
    start(LED_OFF, true);
    send(EVT_BUTTON_SHORT, 0U);
    ticks(MODE_SELECT_WINDOW_BATTERY_TICKS - 2U);
    send(EVT_BUTTON_SHORT, 0U); /* Smart -> Steady inside the window */
    CHECK(sm_current_state() == LED_STEADY);
    ticks(2U); /* a mode change must not have restarted the window */
    send(EVT_BUTTON_SHORT, 0U);
    CHECK(sm_current_state() == IDLE_CHARGING);
}

TEST(test_BTN8_boot_into_active_mode_opens_window)
{
    BEGIN();
    start(LED_SMART, false);
    send(EVT_BUTTON_SHORT, 0U);
    CHECK(sm_current_state() == LED_STEADY);

    /* booting into Idle Charging does not */
    start(IDLE_CHARGING, true);
    ticks(MODE_SELECT_WINDOW_BATTERY_TICKS);
    send(EVT_BUTTON_SHORT, 0U);
    CHECK(sm_current_state() == LED_SMART);
    send(EVT_BUTTON_SHORT, 0U); /* window opened by this entry */
    CHECK(sm_current_state() == LED_STEADY);
}

TEST(test_BTN5_long_press_powers_off)
{
    BEGIN();
    start(LED_STEADY, false);
    send(EVT_BUTTON_LONG, 0U);
    CHECK(sm_is_sleeping());

    start(LED_STEADY, true);
    send(EVT_BUTTON_LONG, 0U);
    CHECK(sm_current_state() == IDLE_CHARGING);
    CHECK(fake.light == LIGHT_PULSE_OFF);

    /* long press in Idle Charging with USB does nothing */
    unsigned int calls = fake.published_calls;
    send(EVT_BUTTON_LONG, 0U);
    CHECK(sm_current_state() == IDLE_CHARGING);
    CHECK(fake.published_calls == calls);
    CHECK(!sm_is_sleeping());
}

TEST(test_BTN6_waits_for_stable_release_before_sleeping)
{
    BEGIN();
    start(LED_STEADY, false);
    fake.press_polls_until_release = 5U; /* still held for five polls */
    send(EVT_BUTTON_LONG, 0U);
    CHECK(sm_is_sleeping());
    CHECK(fake.delayed_ms == (5U + SLEEP_RELEASE_STABLE_POLLS) * SLEEP_RELEASE_POLL_MS);
    CHECK(fake.arm_wake_calls == 1U);
}

TEST(test_BTN6_stuck_button_proceeds_after_limit)
{
    BEGIN();
    start(LED_STEADY, false);
    fake.pressed = true; /* never released */
    send(EVT_BUTTON_LONG, 0U);
    CHECK(sm_is_sleeping());
    CHECK(fake.delayed_ms == SLEEP_RELEASE_WAIT_MS);
}

TEST(test_BTN7_wake_arming_failure_stays_awake_in_off)
{
    BEGIN();
    start(LED_STEADY, false);
    fake.arm_wake_result = -1;
    send(EVT_BUTTON_LONG, 0U);
    CHECK(!sm_is_sleeping());
    CHECK(sm_current_state() == LED_OFF);
    CHECK(fake.light == LIGHT_PULSE_OFF);
    CHECK(fake.reenable_calls == 1U);
    CHECK(fake.sleep_enter_calls == 0U);
    CHECK(fake.battery_tick_running); /* the button still works, ticks go on */
    /* and the button wakes it into the first Active Mode as from Off */
    send(EVT_BUTTON_SHORT, 0U);
    CHECK(sm_current_state() == LED_SMART);
}

/* --- State machine: power states ----------------------------------------- */

TEST(test_PWR1_deep_sleep_stops_everything)
{
    BEGIN();
    start(LED_SMART, false);
    CHECK(fake.sensors_running);
    send(EVT_BUTTON_LONG, 0U);
    CHECK(sm_is_sleeping());
    CHECK(fake.light == LIGHT_PULSE_OFF);
    CHECK(fake.status_off_calls == 1U);
    CHECK(!fake.sensors_running);
    CHECK(!fake.battery_tick_running);
    CHECK(fake.sleep_enter_calls == 1U);
    CHECK(fake.published == LED_OFF);

    start(LED_FLASH, false);
    CHECK(fake.flash_running);
    send(EVT_BUTTON_LONG, 0U);
    CHECK(!fake.flash_running);
}

TEST(test_PWR1_sleeping_ignores_everything_but_wake_sources)
{
    BEGIN();
    start(LED_STEADY, false);
    send(EVT_BUTTON_LONG, 0U);
    send(EVT_BUTTON_SHORT, 0U);
    send(EVT_BATTERY_TICK, 0U);
    send(EVT_REMOTE_SET_STATE, LED_STEADY);
    send(EVT_BRAKE_START, 0U);
    CHECK(sm_is_sleeping());
    CHECK(fake.battery_samples == 0U);
    CHECK(fake.sleep_exit_calls == 0U);
}

TEST(test_BOOT4_button_wake_opens_window_and_reenables_edges)
{
    BEGIN();
    start(LED_STEADY, false);
    send(EVT_BUTTON_LONG, 0U);
    send(EVT_BUTTON_WAKE, 0U);
    CHECK(!sm_is_sleeping());
    CHECK(sm_current_state() == sm_first_active_mode());
    CHECK(fake.sleep_exit_calls == 1U);
    CHECK(fake.reenable_calls == 1U);
    CHECK(fake.battery_tick_running);
    /* the wake press only wakes: no mode step, the selection window is open */
    CHECK(fake.light == LIGHT_PULSE_OFF); /* Smart starts dark */
    send(EVT_BUTTON_SHORT, 0U);
    CHECK(sm_current_state() == LED_STEADY);
}

TEST(test_BOOT3_usb_wakes_into_idle_charging)
{
    BEGIN();
    start(LED_STEADY, false);
    send(EVT_BUTTON_LONG, 0U);
    send(EVT_USB_CONNECTED, 0U);
    CHECK(!sm_is_sleeping());
    CHECK(sm_current_state() == IDLE_CHARGING);
    CHECK(fake.status_charging);
}

TEST(test_BOOT3_usb_removed_while_sleeping_is_tracked)
{
    BEGIN();
    start(LED_STEADY, true);
    send(EVT_BUTTON_LONG, 0U); /* Idle Charging */
    send(EVT_USB_DISCONNECTED, 0U); /* USB-3 -> Deep Sleep */
    CHECK(sm_is_sleeping());
    send(EVT_USB_CONNECTED, 0U);
    CHECK(sm_current_state() == IDLE_CHARGING);
    send(EVT_USB_DISCONNECTED, 0U);
    CHECK(sm_is_sleeping());
    send(EVT_BUTTON_WAKE, 0U);
    CHECK(!sm_usb_present());
}

TEST(test_BOOT4_reenable_failure_resets)
{
    BEGIN();
    start(LED_STEADY, false);
    send(EVT_BUTTON_LONG, 0U);
    fake.reenable_result = -1;
    send(EVT_BUTTON_WAKE, 0U);
    CHECK(fake.resets == 1U);
}

TEST(test_BOOT4_stale_wake_event_when_awake_is_ignored)
{
    BEGIN();
    start(LED_STEADY, false);
    unsigned int calls = fake.published_calls;
    send(EVT_BUTTON_WAKE, 0U);
    CHECK(sm_current_state() == LED_STEADY);
    CHECK(fake.published_calls == calls);
}

TEST(test_PWR2_PWR3_off_only_by_request_and_dark)
{
    BEGIN();
    start(LED_STEADY, false);
    send(EVT_REMOTE_SET_STATE, LED_OFF);
    CHECK(sm_current_state() == LED_STEADY); /* stable window not over */
    settle();
    send(EVT_REMOTE_SET_STATE, LED_OFF);
    CHECK(sm_current_state() == LED_OFF);
    CHECK(fake.light == LIGHT_PULSE_OFF);
    CHECK(!sm_is_sleeping()); /* ANT-10: stays awake, no timeout */
    CHECK(fake.sleep_enter_calls == 0U);
    ticks(100U);
    CHECK(sm_current_state() == LED_OFF);
    CHECK(!sm_is_sleeping());
}

TEST(test_PWR4_all_power_off_paths)
{
    BEGIN();

    /* critical battery, no USB: Deep Sleep, in every state */
    for (enum system_state s = LED_OFF; s <= LED_SMART; s++) {
        start(s, false);
        fake.battery = BATTERY_CRITICAL;
        ticks(1U);
        CHECK(sm_is_sleeping());
    }

    /* with USB: Idle Charging */
    start(LED_STEADY, true);
    fake.battery = BATTERY_CRITICAL;
    ticks(1U);
    CHECK(sm_current_state() == IDLE_CHARGING);

    /* Smart auto-off */
    start(LED_SMART, false);
    send(EVT_STATIONARY_TIMEOUT, 0U);
    CHECK(sm_is_sleeping());
    start(LED_SMART, true);
    send(EVT_STATIONARY_TIMEOUT, 0U);
    CHECK(sm_current_state() == IDLE_CHARGING);
}

TEST(test_BAT7_wake_with_critical_battery_turns_light_on)
{
    BEGIN();
    start(LED_STEADY, false);
    fake.battery = BATTERY_CRITICAL;
    ticks(1U);
    CHECK(sm_is_sleeping());
    send(EVT_BUTTON_WAKE, 0U);
    CHECK(sm_current_state() == sm_first_active_mode()); /* on until the next critical sample */
    ticks(1U);
    CHECK(sm_is_sleeping());
}

/* --- State machine: light output ------------------------------------------ */

TEST(test_LED1_LED2_steady_and_flash)
{
    BEGIN();
    start(LED_STEADY, false);
    CHECK(fake.light == LIGHT_PULSE_BASE);

    start(LED_FLASH, false);
    CHECK(fake.light == LIGHT_PULSE_BASE);
    CHECK(fake.flash_running && fake.flash_ms == FLASH_CYCLE_MS); /* first flash one cycle later */

    send(EVT_FLASH_STEP, 0U);
    CHECK(fake.light == LIGHT_PULSE_PEAK && fake.flash_ms == FLASH_STEP_MS);
    send(EVT_FLASH_STEP, 0U);
    CHECK(fake.light == LIGHT_PULSE_BASE && fake.flash_ms == FLASH_STEP_MS);
    send(EVT_FLASH_STEP, 0U);
    CHECK(fake.light == LIGHT_PULSE_PEAK && fake.flash_ms == FLASH_STEP_MS);
    send(EVT_FLASH_STEP, 0U);
    CHECK(fake.light == LIGHT_PULSE_BASE && fake.flash_ms == FLASH_REST_MS);
    send(EVT_FLASH_STEP, 0U); /* the next cycle starts with a flash again */
    CHECK(fake.light == LIGHT_PULSE_PEAK);
    CHECK(3U * FLASH_STEP_MS + FLASH_REST_MS == FLASH_CYCLE_MS);
}

TEST(test_LED4_flash_step_outside_flash_is_ignored)
{
    BEGIN();
    start(LED_STEADY, false);
    unsigned int calls = fake.light_calls;
    send(EVT_FLASH_STEP, 0U);
    CHECK(fake.light_calls == calls);

    /* leaving Flash stops the timer; a step already queued is stale */
    start(LED_FLASH, true);
    settle();
    send(EVT_REMOTE_SET_STATE, LED_STEADY);
    CHECK(!fake.flash_running);
    calls = fake.light_calls;
    send(EVT_FLASH_STEP, 0U);
    CHECK(fake.light_calls == calls);
}

/* --- State machine: Smart -------------------------------------------------- */

TEST(test_SMART1_SMART2_entering_smart_and_priority)
{
    BEGIN();
    start(LED_SMART, false);
    CHECK(fake.light == LIGHT_PULSE_OFF);
    CHECK(fake.sensors_running && fake.sensors_starts == 1U);

    send(EVT_AMBIENT_DARK, 0U);
    CHECK(fake.light == LIGHT_PULSE_BASE);
    send(EVT_BRAKE_START, 0U);
    CHECK(fake.light == LIGHT_PULSE_PEAK);
    send(EVT_AMBIENT_BRIGHT, 0U); /* braking still wins */
    CHECK(fake.light == LIGHT_PULSE_PEAK);
    send(EVT_BRAKE_STOP, 0U);
    CHECK(fake.light == LIGHT_PULSE_OFF);
    send(EVT_AMBIENT_DARK, 0U);
    send(EVT_BRAKE_START, 0U);
    send(EVT_BRAKE_STOP, 0U);
    CHECK(fake.light == LIGHT_PULSE_BASE);
}

TEST(test_SMART1_flags_cleared_on_reentry_and_sampling_stops)
{
    BEGIN();
    start(LED_SMART, false);
    send(EVT_AMBIENT_DARK, 0U);
    send(EVT_BRAKE_START, 0U);
    CHECK(fake.light == LIGHT_PULSE_PEAK);

    send(EVT_BUTTON_SHORT, 0U); /* Steady (selection window is open from boot) */
    CHECK(sm_current_state() == LED_STEADY);
    CHECK(!fake.sensors_running); /* sampling stops when Smart is left */

    /* re-entering Smart later starts with clean flags */
    start(LED_STEADY, false);
    settle();
    send(EVT_REMOTE_SET_STATE, LED_SMART);
    CHECK(sm_current_state() == LED_SMART);
    CHECK(fake.light == LIGHT_PULSE_OFF); /* braking and dark flags are cleared */
    CHECK(fake.sensors_running);
}

TEST(test_SMART6_sensor_events_outside_smart_are_ignored)
{
    BEGIN();
    start(LED_STEADY, false);
    unsigned int calls = fake.light_calls;
    send(EVT_BRAKE_START, 0U);
    send(EVT_AMBIENT_DARK, 0U);
    send(EVT_BRAKE_STOP, 0U);
    send(EVT_AMBIENT_BRIGHT, 0U);
    send(EVT_STATIONARY_TIMEOUT, 0U);
    CHECK(fake.light_calls == calls);
    CHECK(sm_current_state() == LED_STEADY);

    start(LED_OFF, false);
    send(EVT_STATIONARY_TIMEOUT, 0U);
    CHECK(!sm_is_sleeping());
}

/* --- State machine: USB ----------------------------------------------------- */

TEST(test_USB2_USB3_USB4_usb_edges)
{
    BEGIN();
    start(LED_STEADY, false);
    send(EVT_USB_CONNECTED, 0U);
    CHECK(sm_current_state() == LED_STEADY); /* USB-2: only the status LED */
    CHECK(fake.status_charging && fake.status_on);
    send(EVT_USB_DISCONNECTED, 0U);
    CHECK(sm_current_state() == LED_STEADY);
    CHECK(!fake.status_charging);

    start(IDLE_CHARGING, true);
    send(EVT_USB_DISCONNECTED, 0U); /* USB-3 */
    CHECK(sm_is_sleeping());

    start(LED_OFF, false);
    send(EVT_USB_CONNECTED, 0U);
    CHECK(sm_current_state() == LED_OFF);
    CHECK(fake.status_charging && !fake.status_on);
}

/* --- State machine: status LED ------------------------------------------------ */

TEST(test_STAT1_status_led_priority)
{
    BEGIN();
    /* awake Off (reachable by the controller): heartbeat, not dark */
    start(LED_OFF, false);
    CHECK(!fake.status_low && !fake.status_charging && !fake.status_on);
    CHECK(fake.status_reachable_off);

    start(LED_STEADY, false);
    CHECK(fake.status_on && !fake.status_low && !fake.status_charging);
    CHECK(!fake.status_reachable_off);

    fake.battery = BATTERY_LOW;
    ticks(1U);
    CHECK(sm_low_battery() && fake.status_low); /* low battery wins */
    send(EVT_USB_CONNECTED, 0U);
    CHECK(fake.status_low && fake.status_charging); /* the driver shows low first */
    fake.battery = BATTERY_OK;
    ticks(1U);
    CHECK(!sm_low_battery() && !fake.status_low);
}

TEST(test_STAT1_heartbeat_only_in_awake_off)
{
    BEGIN();

    /* Off set by the controller: heartbeat */
    start(LED_STEADY, false);
    settle();
    send(EVT_REMOTE_SET_STATE, LED_OFF);
    CHECK(sm_current_state() == LED_OFF);
    CHECK(fake.status_reachable_off && !fake.status_on);

    /* Off with low battery: the low battery pattern is passed along too and wins in the driver */
    fake.battery = BATTERY_LOW;
    ticks(BATTERY_CONSECUTIVE);
    CHECK(fake.status_low && fake.status_reachable_off);

    /* leaving Off ends the heartbeat */
    fake.battery = BATTERY_OK;
    ticks(BATTERY_CONSECUTIVE);
    settle();
    send(EVT_REMOTE_SET_STATE, LED_STEADY);
    CHECK(!fake.status_reachable_off && fake.status_on);

    /* Deep Sleep: nothing shown, also not after the state was published as Off */
    send(EVT_BUTTON_LONG, 0U);
    CHECK(sm_is_sleeping());
    CHECK(!fake.status_reachable_off && !fake.status_on && !fake.status_low &&
          !fake.status_charging);

    /* Idle Charging (USB): charging blink, not the heartbeat */
    start(IDLE_CHARGING, true);
    CHECK(fake.status_charging && !fake.status_reachable_off);

    /* wake from Deep Sleep into an Active Mode: solid, no heartbeat */
    start(LED_STEADY, false);
    send(EVT_BUTTON_LONG, 0U);
    send(EVT_BUTTON_WAKE, 0U);
    CHECK(fake.status_on && !fake.status_reachable_off);
}

/* BTN-7: the wake interrupt cannot be armed, the light stays awake in Off */
TEST(test_STAT1_stays_awake_in_off_shows_heartbeat)
{
    BEGIN();
    start(LED_STEADY, false);
    fake.arm_wake_result = -1;
    send(EVT_BUTTON_LONG, 0U);
    CHECK(!sm_is_sleeping() && sm_current_state() == LED_OFF);
    CHECK(fake.status_reachable_off);
}

TEST(test_BAT5_low_battery_does_not_change_main_led)
{
    BEGIN();
    start(LED_STEADY, false);
    unsigned int calls = fake.light_calls;
    fake.battery = BATTERY_LOW;
    ticks(5U);
    CHECK(fake.light_calls == calls);
    CHECK(sm_current_state() == LED_STEADY);
}

/* --- State machine: controller requests ------------------------------------------ */

TEST(test_ANT7_requests_validated)
{
    BEGIN();
    start(LED_STEADY, false);
    settle();
    send(EVT_REMOTE_SET_STATE, IDLE_CHARGING); /* not requestable */
    CHECK(sm_current_state() == LED_STEADY);
    send(EVT_REMOTE_SET_STATE, 9U); /* out of range */
    CHECK(sm_current_state() == LED_STEADY);
    send(EVT_REMOTE_SET_STATE, 255U);
    CHECK(sm_current_state() == LED_STEADY);

    send(EVT_REMOTE_SET_STATE, LED_FLASH);
    CHECK(sm_current_state() == LED_FLASH);
    settle();
    send(EVT_REMOTE_SET_STATE, LED_SMART);
    CHECK(sm_current_state() == LED_SMART);
    CHECK(fake.sensors_running);
}

TEST(test_ANT8_equal_state_and_stable_window)
{
    BEGIN();
    start(LED_STEADY, false);
    settle();
    unsigned int calls = fake.published_calls;
    send(EVT_REMOTE_SET_STATE, LED_STEADY); /* equals the current state */
    CHECK(fake.published_calls == calls);

    /* boot counts as a state change: requests right after it are ignored */
    start(LED_STEADY, false);
    fake.now += REMOTE_STABLE_WINDOW_MS - 1U;
    send(EVT_REMOTE_SET_STATE, LED_FLASH);
    CHECK(sm_current_state() == LED_STEADY);
    fake.now += 2U;
    send(EVT_REMOTE_SET_STATE, LED_FLASH);
    CHECK(sm_current_state() == LED_FLASH);

    /* a button press is a state change too */
    send(EVT_BUTTON_LONG, 0U);
    send(EVT_BUTTON_WAKE, 0U);
    send(EVT_REMOTE_SET_STATE, LED_STEADY);
    CHECK(sm_current_state() == LED_SMART);
}

TEST(test_ANT9_requests_applied_in_idle_charging)
{
    BEGIN();
    start(IDLE_CHARGING, true);
    settle();
    send(EVT_REMOTE_SET_STATE, LED_STEADY);
    CHECK(sm_current_state() == LED_STEADY);
    CHECK(fake.light == LIGHT_PULSE_BASE);
}

TEST(test_ANT_publication_on_every_change)
{
    BEGIN();
    start(LED_STEADY, false);
    CHECK(fake.published == LED_STEADY);
    settle();
    send(EVT_REMOTE_SET_STATE, LED_FLASH);
    CHECK(fake.published == LED_FLASH);
    send(EVT_BUTTON_LONG, 0U);
    CHECK(fake.published == LED_OFF); /* Deep Sleep is published as Off */
}

int main(void)
{
    test_SYS3_SYS4_queue_order_and_drop();
    test_SYS4_driver_events_are_coalesced();
    test_BAT2_BAT3_BAT4_battery_logic();
    test_ANT12_battery_status_and_percent();
    test_SMART11_sample_counts_round_up();
    test_SMART3_SMART8_braking_start_stop_and_hold();
    test_SMART3_braking_again_within_hold_keeps_light_on();
    test_SMART3_below_threshold_is_not_braking();
    test_SMART4_ambient_hysteresis();
    test_SMART5_stationary_timeout_and_reset();
    test_SMART12_SMART13_light_plan();
    test_BOOT1_BOOT2_boot_policy();
    test_SYS2_mode_cycle_order();
    test_BTN4_short_press_from_off_and_idle_charging();
    test_BTN4_end_of_cycle_goes_to_idle_charging_or_deep_sleep();
    test_BTN8_BTN9_selection_window();
    test_BTN8_window_closes_after_ticks_and_not_restarted_by_mode_change();
    test_BTN8_boot_into_active_mode_opens_window();
    test_BTN5_long_press_powers_off();
    test_BTN6_waits_for_stable_release_before_sleeping();
    test_BTN6_stuck_button_proceeds_after_limit();
    test_BTN7_wake_arming_failure_stays_awake_in_off();
    test_PWR1_deep_sleep_stops_everything();
    test_PWR1_sleeping_ignores_everything_but_wake_sources();
    test_BOOT4_button_wake_opens_window_and_reenables_edges();
    test_BOOT3_usb_wakes_into_idle_charging();
    test_BOOT3_usb_removed_while_sleeping_is_tracked();
    test_BOOT4_reenable_failure_resets();
    test_BOOT4_stale_wake_event_when_awake_is_ignored();
    test_PWR2_PWR3_off_only_by_request_and_dark();
    test_PWR4_all_power_off_paths();
    test_BAT7_wake_with_critical_battery_turns_light_on();
    test_LED1_LED2_steady_and_flash();
    test_LED4_flash_step_outside_flash_is_ignored();
    test_SMART1_SMART2_entering_smart_and_priority();
    test_SMART1_flags_cleared_on_reentry_and_sampling_stops();
    test_SMART6_sensor_events_outside_smart_are_ignored();
    test_USB2_USB3_USB4_usb_edges();
    test_STAT1_status_led_priority();
    test_STAT1_heartbeat_only_in_awake_off();
    test_STAT1_stays_awake_in_off_shows_heartbeat();
    test_BAT5_low_battery_does_not_change_main_led();
    test_ANT7_requests_validated();
    test_ANT8_equal_state_and_stable_window();
    test_ANT9_requests_applied_in_idle_charging();
    test_ANT_publication_on_every_change();

    if (failures == 0) {
        printf("all core tests passed\n");
        return 0;
    }
    printf("%d failure(s)\n", failures);
    return 1;
}
