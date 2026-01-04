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
#include "utils.h"

/* GPIO Button Devicetree Specifications */
#define SW0_NODE DT_ALIAS(sw0)

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static struct gpio_callback button_cb_data;

static button_event_handler_t user_cb;
static bool initialized = false;
static bool interrupts_enabled = false;
static uint32_t interrupt_enable_time = 0;
static uint32_t last_event_time = 0;

#define IGNORE_INTERRUPTS_MS 200  /* Ignore spurious interrupts for 200ms after enabling */
#define DEBOUNCE_MS 50  /* Minimum time between button events to filter bounce */

/**
 * @brief GPIO interrupt callback for button press/release
 * 
 * Called when button state changes (press or release).
 * Detects the current state and triggers the user callback.
 */
static void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);
    
    if (!initialized || !user_cb) {
        return;
    }
    
    /* Ignore interrupts for a short time after enabling to filter spurious events */
    if (!interrupts_enabled) {
        debug_printk("Button interrupt before enabled flag - ignoring\n");
        return;
    }
    
    uint32_t current_time = k_uptime_get_32();
    if (current_time - interrupt_enable_time < IGNORE_INTERRUPTS_MS) {
        debug_printk("Ignoring spurious interrupt during init window (age: %u ms)\n", 
                     current_time - interrupt_enable_time);
        return;
    }
    
    /* Debounce: ignore events that occur too quickly after the last one */
    if (current_time - last_event_time < DEBOUNCE_MS) {
        debug_printk("Debouncing: ignoring event %u ms after last event\n", 
                     current_time - last_event_time);
        return;
    }
    
    /* Update last event time for debouncing */
    last_event_time = current_time;
    
    /* Read current button state */
    int val = gpio_pin_get_dt(&button);
    if (val < 0) {
        debug_printk("Error reading button state: %d\n", val);
        return;
    }
    
    /* Button is active low, so val == 0 means pressed */
    enum button_evt evt = (val == 0) ? BUTTON_EVT_PRESSED : BUTTON_EVT_RELEASED;
    
    if (evt == BUTTON_EVT_PRESSED) {
        debug_printk("Button pressed\n");
    } else {
        debug_printk("Button released\n");
    }
    
    user_cb(evt);
}

/**
 * @brief Initialize GPIO button with interrupt
 * 
 * Configures the button GPIO pin and sets up interrupt for state changes.
 * 
 * @param handler Function to call when button event is detected
 * @return 0 on success, negative error code on failure
 */
int button_init(button_event_handler_t handler)
{
    int err;

    if (!handler) {
        return -EINVAL;
    }

    user_cb = handler;

    /* Check if GPIO device is ready */
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

    /* Wait for button to be released before enabling interrupts
     * This prevents the wake-up button press from interfering with normal operation.
     * When waking from sleep, the button is still pressed during initialization.
     * We wait up to 1 second for release, checking every 50ms. */
    debug_printk("Waiting for button release before enabling interrupts...\n");
    int timeout_count = 0;
    const int max_timeout = 20;  /* 20 * 50ms = 1 second */
    while (timeout_count < max_timeout) {
        int button_state = gpio_pin_get_dt(&button);
        if (button_state > 0) {
            /* Button is released (high = not pressed, since active low) */
            debug_printk("Button released, waiting additional 100ms for debounce\n");
            k_msleep(100);  /* Extra delay for debounce */
            break;
        }
        k_msleep(50);
        timeout_count++;
    }
    
    if (timeout_count >= max_timeout) {
        debug_printk("Warning: Button still pressed after 1 second timeout\n");
    }

    /* Mark as initialized - interrupts will be enabled later */
    initialized = true;
    interrupts_enabled = false;
    
    debug_printk("GPIO button configured successfully on %s pin %d (interrupts not yet enabled)\n", 
                 button.port->name, button.pin);
    
    return 0;
}

/**
 * @brief Enable button interrupts after initialization
 * 
 * Should be called after all slow initialization tasks (e.g., I2C scanning)
 * to avoid spurious interrupts during boot/wake-up.
 * 
 * @return 0 on success, negative error code on failure
 */
int button_enable_interrupts(void)
{
    int err;
    
    if (!initialized) {
        debug_printk("Button not initialized, call button_init() first\n");
        return -EINVAL;
    }
    
    if (interrupts_enabled) {
        debug_printk("Button interrupts already enabled\n");
        return 0;
    }
    
    /* Check if GPIO device is ready */
    if (!gpio_is_ready_dt(&button)) {
        debug_printk("Button GPIO device %s is not ready\n", button.port->name);
        return -EIO;
    }
    
    /* Clear any pending interrupts by disabling and re-enabling */
    debug_printk("Clearing pending interrupts...\n");
    err = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_DISABLE);
    if (err < 0) {
        debug_printk("Error %d: failed to disable interrupt on %s pin %d\n", 
                     err, button.port->name, button.pin);
        return err;
    }
    
    /* Small delay for hardware to settle */
    k_msleep(10);
    
    /* Re-enable interrupt on rising edge (button release for active-low) */
    err = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_RISING);
    if (err < 0) {
        debug_printk("Error %d: failed to configure interrupt on %s pin %d\n", 
                     err, button.port->name, button.pin);
        return err;
    }

    /* Initialize and add callback */
    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);
    
    /* Mark interrupts as enabled and record time */
    interrupts_enabled = true;
    interrupt_enable_time = k_uptime_get_32();
    last_event_time = interrupt_enable_time;
    
    debug_printk("Button interrupts enabled successfully on %s pin %d\n", 
                 button.port->name, button.pin);
    
    return 0;
}

/**
 * @brief Configure button as wakeup source for system off mode
 * 
 * Reconfigures the button interrupt to use level-active triggering,
 * which is required to wake the system from system off (deep sleep).
 * 
 * @return 0 on success, negative error code on failure
 */
int button_configure_wakeup(void)
{
    int err;
    
    if (!initialized) {
        debug_printk("Button not initialized, cannot configure for wakeup\n");
        return -EINVAL;
    }
    
    /* Check if GPIO device is ready */
    if (!gpio_is_ready_dt(&button)) {
        debug_printk("Button GPIO device %s is not ready\n", button.port->name);
        return -EIO;
    }
    
    /* Configure interrupt as level-active to wake from system off */
    err = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_LEVEL_ACTIVE);
    if (err < 0) {
        debug_printk("Error %d: failed to configure wakeup interrupt on %s pin %d\n", 
                     err, button.port->name, button.pin);
        return err;
    }
    
    debug_printk("Button configured as wakeup source (level-active interrupt)\n");
    
    return 0;
}
