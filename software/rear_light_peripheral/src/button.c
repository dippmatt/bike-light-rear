/*
 * Copyright (c) 2025 Matthias Dippold
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * NON-COMMERCIAL USE CLAUSE:
 * Permission is granted to use, copy, modify, and distribute this software
 * and its derivatives for NON-COMMERCIAL purposes only. Commercial use of
 * this software, in whole or in part, is strictly prohibited without
 * explicit written permission from the copyright holder.
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>

#include "button.h"
#include "app_events.h"
#include "debug.h"

#define SW0_NODE DT_ALIAS(sw0)

#define DEBOUNCE_MS 200 /* Minimum time between button events to filter bounce */

#define SAMPLE_INTERVAL_MS 100 /* Press-length sampling interval */
#define LONG_PRESS_SAMPLES 10  /* Held for 10 samples (1 s) -> long press */

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static struct gpio_callback button_cb_data;
static struct gpio_callback wake_cb_data;

static bool initialized;
static bool callback_added;
static bool wake_callback_added;
static uint32_t last_event_time;

/* Press-length detection: samples the pin every 100 ms while pressed */
static struct k_timer sample_timer;
static uint8_t press_counter;
static bool timer_active;

/**
 * @brief Sample timer expiry (ISR context)
 *
 * Samples the button every 100 ms after a press edge.
 * Released before 1 s -> short press; still held at 1 s -> long press.
 */
static void sample_timer_expiry(struct k_timer *timer)
{
    ARG_UNUSED(timer);

    if (!timer_active) {
        return;
    }

    int val = gpio_pin_get_dt(&button);
    if (val < 0) {
        debug_printk("Error reading button state during sampling: %d\n", val);
        /* Continue sampling anyway */
        return;
    }

    /* val == 1 means pressed, DT handles active low / active high differentiation */
    if (val != 1) {
        /* Released before the long-press threshold -> short press */
        k_timer_stop(&sample_timer);
        timer_active = false;
        press_counter = 0;

        debug_printk("Button released - short press\n");
        app_event_post(EVT_BUTTON_SHORT, 0);
        return;
    }

    press_counter++;
    if (press_counter >= LONG_PRESS_SAMPLES) {
        /* Held for 1 s -> long press, fires while still held */
        k_timer_stop(&sample_timer);
        timer_active = false;
        press_counter = 0;

        debug_printk("Button held - long press\n");
        app_event_post(EVT_BUTTON_LONG, 0);
    }
}

/**
 * @brief One-shot level interrupt callback used while in Deep Sleep
 *
 * A level-active interrupt re-fires for as long as the button is held, so
 * the interrupt is disarmed first and the wake request is posted once.
 */
static void wake_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    (void)gpio_pin_interrupt_configure_dt(&button, GPIO_INT_DISABLE);
    app_event_post(EVT_BUTTON_WAKE, 0);
}

/**
 * @brief GPIO interrupt callback for button press/release edges
 */
static void button_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    if (!initialized) {
        return;
    }

    /* We use EDGE_BOTH so we see both press and release; only act on press
     * (pin read). This avoids losing the first user press when the driver
     * "consumes" the first edge after switching from level (wake) to edge
     * (normal) on second boot. */
    int val = gpio_pin_get_dt(&button);
    if (val <= 0) {
        /* Released or read error - ignore (we only start the timer on press) */
        return;
    }

    /* Debounce: ignore presses that occur too quickly after the last one */
    uint32_t now = k_uptime_get_32();
    if (now - last_event_time < DEBOUNCE_MS) {
        debug_printk("Debouncing: ignoring event %u ms after last event\n",
                     now - last_event_time);
        return;
    }
    last_event_time = now;

    /* Restart press-length sampling */
    if (timer_active) {
        k_timer_stop(&sample_timer);
    }
    press_counter = 0;
    timer_active = true;
    k_timer_start(&sample_timer, K_MSEC(SAMPLE_INTERVAL_MS), K_MSEC(SAMPLE_INTERVAL_MS));

    debug_printk("Button pressed - starting press-length sampling\n");
}

int button_init(void)
{
    int err;

    if (!gpio_is_ready_dt(&button)) {
        debug_printk("Button GPIO device %s is not ready\n", button.port->name);
        return -EIO;
    }

    /* Configure button pin as input with pull-up (active low is set in device tree) */
    err = gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_PULL_UP);
    if (err < 0) {
        debug_printk("Error %d: failed to configure %s pin %d\n",
                     err, button.port->name, button.pin);
        return err;
    }

    k_timer_init(&sample_timer, sample_timer_expiry, NULL);
    gpio_init_callback(&button_cb_data, button_isr, BIT(button.pin));
    gpio_init_callback(&wake_cb_data, wake_isr, BIT(button.pin));

    initialized = true;

    debug_printk("Button GPIO configured on %s pin %d (interrupts not yet enabled)\n",
                 button.port->name, button.pin);
    return 0;
}

/**
 * @brief Register the callback and enable the edge interrupt
 *
 * Shared implementation for button_enable() and button_reenable().
 */
static int enable_edge_interrupt(void)
{
    int err;

    if (!initialized) {
        debug_printk("Button not initialized, call button_init() first\n");
        return -EINVAL;
    }

    /* Clear any pending interrupt by disabling first */
    err = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_DISABLE);
    if (err < 0) {
        debug_printk("Error %d: failed to disable button interrupt\n", err);
        return err;
    }

    if (wake_callback_added) {
        gpio_remove_callback(button.port, &wake_cb_data);
        wake_callback_added = false;
    }

    if (!callback_added) {
        gpio_add_callback(button.port, &button_cb_data);
        callback_added = true;
    }

    /* Use BOTH edges so we see press and release; the callback only acts on
     * press (pin read). Avoids losing the first user press when nRF52/driver
     * consumes the first edge after switching from level (wake) to edge. */
    err = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_BOTH);
    if (err < 0) {
        debug_printk("Error %d: failed to configure button edge interrupt\n", err);
        return err;
    }

    last_event_time = k_uptime_get_32();

    debug_printk("Button interrupts enabled on %s pin %d\n",
                 button.port->name, button.pin);
    return 0;
}

int button_enable(void)
{
    return enable_edge_interrupt();
}

int button_reenable(void)
{
    return enable_edge_interrupt();
}

bool button_is_pressed(void)
{
    return gpio_pin_get_dt(&button) == 1;
}

int button_arm_wake(void)
{
    int err;

    if (!initialized) {
        debug_printk("Button not initialized, cannot arm wake\n");
        return -EINVAL;
    }

    /* 1. Stop press-length sampling to prevent further ISR-context work */
    if (timer_active) {
        k_timer_stop(&sample_timer);
        timer_active = false;
        press_counter = 0;
    }

    /* 2. Disable the current edge-triggered interrupt */
    err = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_DISABLE);
    if (err < 0) {
        debug_printk("Error %d: failed to disable interrupt before wake arming\n", err);
        return err;
    }

    /* 3. Swap the press callback for the one-shot wake callback */
    if (callback_added) {
        gpio_remove_callback(button.port, &button_cb_data);
        callback_added = false;
    }

    if (!wake_callback_added) {
        gpio_add_callback(button.port, &wake_cb_data);
        wake_callback_added = true;
    }

    /* 4. Level-active uses the nRF SENSE / PORT event: no GPIOTE channel
     *    and no extra current while the CPU sleeps. */
    err = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_LEVEL_ACTIVE);
    if (err < 0) {
        debug_printk("Error %d: failed to arm wake interrupt\n", err);
        return err;
    }

    debug_printk("Button armed as wake source (level-active, one-shot)\n");
    return 0;
}
