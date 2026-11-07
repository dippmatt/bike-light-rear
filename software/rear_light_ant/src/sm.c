/*
 * Central state machine, see sm.h.
 */

#include "sm.h"

#include "config.h"

/* Mode Cycle: the Active Modes in short-press order. This is the only place
 * that knows the order - the end of the cycle (Idle Charging or Deep Sleep)
 * and the entry point (first mode) are derived from it (SYS-2). */
static const enum system_state mode_cycle[] = {
    LED_SMART,
    LED_STEADY,
    LED_FLASH,
};

#define MODE_CYCLE_LEN (sizeof(mode_cycle) / sizeof(mode_cycle[0]))

/* All of the following is owned exclusively by the main loop */
static const struct sm_ops *ops;
static enum system_state state;
static uint32_t last_change_ms;

/* Battery ticks since the light was last switched on from an inactive
 * state; saturates at MODE_SELECT_WINDOW_BATTERY_TICKS (window closed) */
static uint8_t selection_ticks = MODE_SELECT_WINDOW_BATTERY_TICKS;

/* True while in Deep Sleep: only wake requests and USB edges are handled */
static bool sleeping;

static bool usb_present;
static bool low_battery;
static bool braking;
static bool ambient_dark;
static uint8_t flash_phase;

/* Smart is parked: LED off, only motion is watched (SMART-16) */
static bool parked;

/* Position of a state in the Mode Cycle, or -1 if it is not an Active Mode */
static int cycle_index(enum system_state s)
{
    for (unsigned int i = 0; i < MODE_CYCLE_LEN; i++) {
        if (mode_cycle[i] == s) {
            return (int)i;
        }
    }

    return -1;
}

static bool light_is_on(enum system_state s)
{
    return cycle_index(s) >= 0;
}

/* Both Smart variants share brightness and sensor handling */
static bool is_smart(enum system_state s)
{
    return s == LED_SMART || s == LED_SMART_AUTO_OFF;
}

static void status_refresh(void)
{
    /* Off while awake is reachable; Deep Sleep (sleeping) shows nothing */
    ops->status_set(low_battery, usb_present, light_is_on(state),
                    state == LED_OFF && !sleeping, parked);
}

/* Smart brightness priority: braking > ambient dark > off (SMART-2) */
static void apply_smart_pwm(void)
{
    if (braking) {
        ops->light_set(LIGHT_PULSE_PEAK);
    } else if (ambient_dark) {
        ops->light_set(LIGHT_PULSE_BASE);
    } else {
        ops->light_set(LIGHT_PULSE_OFF);
    }
}

static void state_enter(enum system_state s)
{
    switch (s) {
    case LED_OFF:
    case IDLE_CHARGING:
        ops->light_set(LIGHT_PULSE_OFF);
        break;
    case LED_STEADY:
        ops->light_set(LIGHT_PULSE_BASE);
        break;
    case LED_FLASH:
        ops->light_set(LIGHT_PULSE_BASE);
        flash_phase = 0U;
        ops->flash_timer_start(FLASH_CYCLE_MS); /* first double flash one cycle later */
        break;
    case LED_SMART:
    case LED_SMART_AUTO_OFF:
        braking = false;
        ambient_dark = false;
        ops->light_set(LIGHT_PULSE_OFF);
        ops->sensors_start();
        break;
    default:
        break;
    }
}

static void state_exit(enum system_state s)
{
    switch (s) {
    case LED_FLASH:
        ops->flash_timer_stop();
        break;
    case LED_SMART:
    case LED_SMART_AUTO_OFF:
        ops->sensors_stop(); /* stops parked sampling too */
        parked = false;
        break;
    default:
        break;
    }
}

static void transition(enum system_state to)
{
    bool was_active;

    if (to == state) {
        return;
    }

    was_active = light_is_on(state);

    state_exit(state);
    state = to;
    state_enter(to);

    last_change_ms = ops->now_ms();

    /* Only an inactive -> Active Mode transition opens the selection window;
     * changes between Active Modes neither open nor restart it (BTN-8) */
    if (!was_active && light_is_on(to)) {
        selection_ticks = 0U;
    }

    ops->state_changed(to);
    status_refresh();
}

/* Deep Sleep is System ON with all outputs, sensors, the ANT channel and the
 * battery tick stopped (ADR-0004). Called only from the main loop,
 * synchronously. If the wake interrupt cannot be armed, the device stays
 * awake in Off with a working button instead of ending up with dead inputs
 * (BTN-7). */
static void do_deep_sleep(void)
{
    uint8_t released_polls = 0U;

    state_exit(state); /* stops flash timer / sensor sampling */
    state = LED_OFF;
    ops->light_set(LIGHT_PULSE_OFF);
    ops->status_off();

    /* Wait for a stable button release: the long press that triggered this
     * fires while the button is still held. A genuinely stuck button
     * proceeds after the timeout and causes a visible wake instead of a
     * dead device (BTN-6). */
    for (uint32_t i = 0U; i < SLEEP_RELEASE_WAIT_MS / SLEEP_RELEASE_POLL_MS &&
                          released_polls < SLEEP_RELEASE_STABLE_POLLS; i++) {
        released_polls = ops->button_is_pressed() ? 0U : (uint8_t)(released_polls + 1U);
        ops->delay_ms(SLEEP_RELEASE_POLL_MS);
    }

    if (ops->button_arm_wake() != 0) {
        (void)ops->button_reenable();
        last_change_ms = ops->now_ms();
        ops->state_changed(LED_OFF);
        status_refresh();
        return;
    }

    ops->battery_tick_stop();
    low_battery = false;
    sleeping = true;
    ops->state_changed(LED_OFF);
    ops->sleep_enter();
}

/* Power the light off: Idle Charging with USB power, else Deep Sleep (PWR-4) */
static void power_off(void)
{
    if (usb_present) {
        transition(IDLE_CHARGING);
    } else {
        do_deep_sleep();
    }
}

/* Leave Deep Sleep into the given state, as a boot would (BOOT-3, BOOT-4) */
static void wake_from_sleep(enum system_state to)
{
    sleeping = false;

    ops->sleep_exit();

    /* A button that is still held produces no event; only a new press edge
     * counts (the wake press only wakes). */
    if (ops->button_reenable() != 0) {
        ops->delay_ms(100U);
        ops->reset();
        return;
    }

    ops->battery_tick_start();

    transition(to);
}

/* While sleeping, everything is ignored except the wake sources. USB edges
 * are still tracked (the poll timer keeps running), so the wake state and
 * the status LED are right afterwards. */
static void handle_sleeping_event(const struct app_event *evt)
{
    switch (evt->type) {
    case EVT_BUTTON_WAKE:
        wake_from_sleep(mode_cycle[0]);
        break;
    case EVT_USB_CONNECTED:
        usb_present = true;
        wake_from_sleep(IDLE_CHARGING);
        break;
    case EVT_USB_DISCONNECTED:
        usb_present = false;
        break;
    default:
        break;
    }
}

static void handle_button_short(void)
{
    int idx = cycle_index(state);

    if (idx < 0) {
        /* Off or Idle Charging: enter the first Active Mode */
        transition(mode_cycle[0]);
    } else if (selection_ticks < MODE_SELECT_WINDOW_BATTERY_TICKS &&
               (unsigned int)idx + 1U < MODE_CYCLE_LEN) {
        /* Inside the selection window: next Active Mode */
        transition(mode_cycle[idx + 1]);
    } else if (usb_present) {
        /* After the last Active Mode, or after the selection window closed */
        transition(IDLE_CHARGING);
    } else {
        do_deep_sleep();
    }
}

/* No motion for the stationary timeout: the earlier Smart powers off, Smart
 * parks (SMART-5, SMART-16). If the motion detector cannot be set up, Smart
 * keeps full sampling instead of going dark without a wake source (SMART-19). */
static void handle_stationary_timeout(void)
{
    if (state == LED_SMART_AUTO_OFF) {
        power_off();
        return;
    }
    if (state != LED_SMART || parked) {
        return;
    }

    ops->sensors_stop();
    if (ops->sensors_start_parked() != 0) {
        ops->sensors_start();
        return;
    }

    parked = true;
    braking = false;
    ambient_dark = false;
    ops->light_set(LIGHT_PULSE_OFF);
    status_refresh();
}

/* Motion while parked: back to full Smart, as on entering it (SMART-17).
 * Not a state change: no publication, the Selection Window is untouched. */
static void handle_motion_detected(void)
{
    if (state != LED_SMART || !parked) {
        return;
    }

    ops->sensors_stop();
    parked = false;
    ops->sensors_start();
    status_refresh();
}

/* A request for Smart (ANT+ Auto) selects the Smart variant in the Mode
 * Cycle, so swapping the variants is one edit of mode_cycle (ANT-3) */
static uint8_t smart_in_cycle(void)
{
    for (unsigned int i = 0; i < MODE_CYCLE_LEN; i++) {
        if (is_smart(mode_cycle[i])) {
            return (uint8_t)mode_cycle[i];
        }
    }
    return (uint8_t)LED_SMART;
}

static void handle_remote_set_state(uint8_t requested)
{
    if (requested == (uint8_t)LED_SMART) {
        requested = smart_in_cycle();
    }

    /* Only Off and the Active Modes can be requested; Idle Charging is set
     * by USB power alone. Invalid requests are ignored (ANT-7). */
    if (requested != (uint8_t)LED_OFF && cycle_index((enum system_state)requested) < 0) {
        return;
    }

    if (requested == (uint8_t)state) {
        return;
    }

    /* A remote cannot fight a just-pressed button (ANT-8) */
    if (ops->now_ms() - last_change_ms < REMOTE_STABLE_WINDOW_MS) {
        return;
    }

    /* A requested Off keeps the system awake and reachable (ANT-10); only
     * the button and the auto-off paths power the system down */
    transition((enum system_state)requested);
}

static void handle_flash_step(void)
{
    if (state != LED_FLASH) {
        return; /* stale event from a stopped timer (LED-4) */
    }

    switch (flash_phase) {
    case 0U:
        ops->light_set(LIGHT_PULSE_PEAK);
        flash_phase = 1U;
        ops->flash_timer_start(FLASH_STEP_MS);
        break;
    case 1U:
        ops->light_set(LIGHT_PULSE_BASE);
        flash_phase = 2U;
        ops->flash_timer_start(FLASH_STEP_MS);
        break;
    case 2U:
        ops->light_set(LIGHT_PULSE_PEAK);
        flash_phase = 3U;
        ops->flash_timer_start(FLASH_STEP_MS);
        break;
    default:
        ops->light_set(LIGHT_PULSE_BASE);
        flash_phase = 0U;
        ops->flash_timer_start(FLASH_REST_MS);
        break;
    }
}

static void handle_battery_tick(void)
{
    enum battery_level level;
    bool low;

    /* The battery tick also times the selection window */
    if (selection_ticks < MODE_SELECT_WINDOW_BATTERY_TICKS) {
        selection_ticks++;
    }

    level = ops->battery_sample();

    if (level == BATTERY_CRITICAL) {
        power_off();
        return;
    }

    low = (level == BATTERY_LOW);
    if (low != low_battery) {
        low_battery = low;
        status_refresh();
    }
}

void sm_handle_event(const struct app_event *evt)
{
    if (sleeping) {
        handle_sleeping_event(evt);
        return;
    }

    switch (evt->type) {
    case EVT_BUTTON_SHORT:
        handle_button_short();
        break;
    case EVT_BUTTON_LONG:
        power_off();
        break;
    case EVT_REMOTE_SET_STATE:
        handle_remote_set_state(evt->arg);
        break;
    case EVT_FLASH_STEP:
        handle_flash_step();
        break;
    case EVT_BATTERY_TICK:
        handle_battery_tick();
        break;
    case EVT_USB_CONNECTED:
        usb_present = true;
        status_refresh();
        break;
    case EVT_USB_DISCONNECTED:
        usb_present = false;
        if (state == IDLE_CHARGING) {
            do_deep_sleep();
        } else {
            status_refresh();
        }
        break;
    /* Detector events are stale while parked: the detectors are stopped */
    case EVT_BRAKE_START:
        if (is_smart(state) && !parked) {
            braking = true;
            apply_smart_pwm();
        }
        break;
    case EVT_BRAKE_STOP:
        if (is_smart(state) && !parked) {
            braking = false;
            apply_smart_pwm();
        }
        break;
    case EVT_AMBIENT_DARK:
        if (is_smart(state) && !parked) {
            ambient_dark = true;
            apply_smart_pwm();
        }
        break;
    case EVT_AMBIENT_BRIGHT:
        if (is_smart(state) && !parked) {
            ambient_dark = false;
            apply_smart_pwm();
        }
        break;
    case EVT_BUTTON_WAKE:
        break; /* stale wake request (device is already awake) */
    case EVT_STATIONARY_TIMEOUT:
        handle_stationary_timeout();
        break;
    case EVT_MOTION_DETECTED:
        handle_motion_detected();
        break;
    default:
        break;
    }
}

void sm_init(const struct sm_ops *sm_ops, enum system_state boot_state, bool usb)
{
    ops = sm_ops;

    usb_present = usb;
    low_battery = false;
    braking = false;
    ambient_dark = false;
    sleeping = false;
    parked = false;
    flash_phase = 0U;
    selection_ticks = MODE_SELECT_WINDOW_BATTERY_TICKS;

    state = boot_state;
    state_enter(boot_state);

    last_change_ms = ops->now_ms();

    /* Booting into an Active Mode counts as inactive -> active */
    if (light_is_on(boot_state)) {
        selection_ticks = 0U;
    }

    ops->state_changed(boot_state);
    status_refresh();
}

enum system_state sm_boot_state(bool power_on_reset, bool usb)
{
    return (power_on_reset && usb) ? IDLE_CHARGING : mode_cycle[0];
}

enum system_state sm_first_active_mode(void)
{
    return mode_cycle[0];
}

enum system_state sm_current_state(void)
{
    return state;
}

bool sm_is_sleeping(void)
{
    return sleeping;
}

bool sm_is_parked(void)
{
    return parked;
}

bool sm_usb_present(void)
{
    return usb_present;
}

bool sm_low_battery(void)
{
    return low_battery;
}
