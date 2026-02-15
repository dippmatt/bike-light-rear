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
static uint32_t last_event_time = 0;
static uint32_t current_time = 0;

#define DEBOUNCE_MS 200   /* Minimum time between button events to filter bounce */

/* Long press detection state */
static struct k_timer long_press_timer;
static uint8_t press_counter = 0;
static bool timer_active = false;
static struct k_work button_work;
static enum button_press_type pending_press_type;

static void button_work_handler(struct k_work *work)
{
    ARG_UNUSED(work);

    if (!initialized || !user_cb) {
        return;
    }

    user_cb(pending_press_type);
}

/**
 * @brief Timer expiry callback for long press detection
 * 
 * Periodically samples the button state every 100ms.
 * If button is released before 1s (counter > 10), triggers short press callback.
 * If button is still pressed after 1s (counter > 10), triggers long press callback.
 */
static void long_press_timer_handler(struct k_timer *timer)
{
    ARG_UNUSED(timer);
    
    if (!initialized || !user_cb || !timer_active) {
        return;
    }
    
    /* Read current button state */
    int val = gpio_pin_get_dt(&button);
    if (val < 0) {
        debug_printk("Error reading button state during sampling: %d\n", val);
        /* Continue sampling anyway */
        return;
    }
    
    /* val == 1 means pressed, DT handles active low / active high differentiation */
    if (val != 1) {
        /* Button released - short press detected */
        k_timer_stop(&long_press_timer);
        timer_active = false;
        press_counter = 0;
        
        debug_printk("Button released - short press detected\n");
        pending_press_type = BUTTON_PRESS_SHORT;
        k_work_submit(&button_work);
    } else {
        /* Button still pressed - increment counter */
        press_counter++;
        debug_printk("Button still pressed - counter: %u\n", press_counter);
        
        if (press_counter > 10) {
            /* Long press confirmed (1s elapsed) */
            k_timer_stop(&long_press_timer);
            timer_active = false;
            press_counter = 0;
            
            debug_printk("Long press confirmed after %u samples\n", press_counter);
            pending_press_type = BUTTON_PRESS_LONG;
            k_work_submit(&button_work);
        }
        /* Timer will auto-repeat every 100ms if still active */
    }
}

/**
 * @brief GPIO interrupt callback for button press/release
 * 
 * Called when button is pressed (falling edge for active-low button).
 * Starts periodic timer to detect short vs long press.
 */
static void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);
    
    if (!initialized || !user_cb) {
        return;
    }
    
    current_time = k_uptime_get_32();
    /* We use EDGE_BOTH so we see both press and release; only act on press (pin low).
     * This avoids losing the first user press when the driver "consumes" the first edge
     * after switching from level (wake) to edge (normal) on second boot. */
    int val = gpio_pin_get_dt(&button);
    if (val <= 0) {
        /* Released or read error - ignore (we only start timer on press) */
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
    
    /* Cancel any existing timer (if previous press wasn't handled) */
    if (timer_active) {
        k_timer_stop(&long_press_timer);
        timer_active = false;
    }
    
    /* Reset counter and start periodic sampling timer */
    press_counter = 0;
    timer_active = true;
    k_timer_start(&long_press_timer, K_MSEC(100), K_MSEC(100));
    
    debug_printk("Button pressed - starting long press detection timer\n");
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
    k_work_init(&button_work, button_work_handler);

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

    /* Mark as initialized - interrupts will be enabled later */
    initialized = true;
    
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
    
    /* Initialize long press detection timer */
    k_timer_init(&long_press_timer, long_press_timer_handler, NULL);
    timer_active = false;
    press_counter = 0;
    
    /* Use BOTH edges so we see press and release; callback only acts on press (pin read).
     * Avoids losing first user press when nRF52/driver consumes first edge after wake. */
    err = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_BOTH);
    if (err < 0) {
        debug_printk("Error %d: failed to configure interrupt on %s pin %d\n", 
                     err, button.port->name, button.pin);
        return err;
    }
    
    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);
    
    last_event_time = k_uptime_get_32();
    
    debug_printk("Button interrupts enabled successfully on %s pin %d\n", 
                 button.port->name, button.pin);
    
    return 0;
}

/**
 * @brief Configure button as wakeup source for system off mode
 * 
 * Cleanly shuts down normal button interrupt handling (edge-triggered callback),
 * then configures the pin for level-active triggering required to wake the
 * system from system off (deep sleep).
 * 
 * The GPIO callback is removed BEFORE switching to level-active to prevent
 * an interrupt storm: level-active fires continuously while the pin is at the
 * active level, and if the old edge-triggered callback is still registered it
 * would be invoked on every such interrupt, flooding the system and potentially
 * causing sys_poweroff() to abort (nRF52 aborts System OFF entry if a DETECT
 * signal is already active).
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
    
    /* 1. Stop the long-press detection timer to prevent further ISR-context work */
    if (timer_active) {
        k_timer_stop(&long_press_timer);
        timer_active = false;
    }
    
    /* 2. Disable the current edge-triggered interrupt */
    err = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_DISABLE);
    if (err < 0) {
        debug_printk("Error %d: failed to disable interrupt before wakeup config\n", err);
        return err;
    }
    
    /* 3. Remove the GPIO callback so level-active won't invoke button_pressed */
    gpio_remove_callback(button.port, &button_cb_data);
    
    /* 4. Now safely configure level-active for System OFF wakeup.
     *    The nRF52 SENSE mechanism will wake the chip from System OFF via
     *    hardware DETECT — no software callback is needed for this. */
    err = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_LEVEL_ACTIVE);
    if (err < 0) {
        debug_printk("Error %d: failed to configure wakeup interrupt on %s pin %d\n", 
                     err, button.port->name, button.pin);
        return err;
    }
    
    debug_printk("Button configured as wakeup source (level-active, callback removed)\n");
    
    return 0;
}
