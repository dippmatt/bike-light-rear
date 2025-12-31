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

    /* Configure interrupt for both edges (press and release) */
    err = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_BOTH);
    if (err < 0) {
        debug_printk("Error %d: failed to configure interrupt on %s pin %d\n", 
                     err, button.port->name, button.pin);
        return err;
    }

    /* Initialize and add callback */
    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);
    
    initialized = true;
    
    debug_printk("GPIO button initialized successfully on %s pin %d\n", 
                 button.port->name, button.pin);
    
    return 0;
}
