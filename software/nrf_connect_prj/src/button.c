/*
 * Copyright (c) 2024
 *
 * SPDX-LICENSE-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>

#include "button.h"
#include "utils.h"

/* GPIO Devicetree Specifications */
#define BUTTON_NODE         DT_ALIAS(sw0)

/* Button debounce time in milliseconds */
#define BUTTON_DEBOUNCE_MS 50

/* GPIO device specification for button */
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

/* Debounce timer */
static struct k_timer debounce_timer;

/* Last button state */
static volatile bool last_button_state = false;

/* Button press callback */
static button_press_callback_t button_callback = NULL;

/* GPIO callback structure */
static struct gpio_callback button_cb_data;

/**
 * @brief Debounce timer expiry callback
 * 
 * Checks button state after debounce delay and calls callback on press.
 */
static void debounce_timer_expiry()
{
    /* Read current button state (active low: 0 = pressed, 1 = released) */
    int pin_state = gpio_pin_get_dt(&button);
    bool currently_pressed = (pin_state == 0);
    
    /* Call callback on button press (transition from released to pressed) */
    if (currently_pressed && !last_button_state) {
        debug_printk("Button pressed\n");
        if (button_callback != NULL) {
            button_callback();
        }
    }
    
    last_button_state = currently_pressed;
}

/**
 * @brief GPIO interrupt callback for button
 * 
 * Triggered when button state changes. Starts debounce timer.
 */
static void button_pressed_callback()
{
    /* Start debounce timer */
    k_timer_start(&debounce_timer, K_MSEC(BUTTON_DEBOUNCE_MS), K_NO_WAIT);
}

/**
 * @brief Initialize button hardware and detection
 * 
 * Configures GPIO pin, sets up interrupts, and initializes debounce timer.
 * 
 * @param callback Function to call when button press is detected
 * @return 0 on success, negative error code on failure
 */
int button_init(button_press_callback_t callback)
{
    int ret;
    
    if (!gpio_is_ready_dt(&button)) {
        return -ENODEV;
    }
    
    /* Store callback */
    button_callback = callback;
    
    /* Configure button as input with pullup */
    ret = gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        return ret;
    }
    
    /* Initialize debounce timer */
    k_timer_init(&debounce_timer, debounce_timer_expiry, NULL);
    
    /* Configure button interrupt on both edges */
    ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_BOTH);
    if (ret < 0) {
        return ret;
    }
    
    /* Initialize and add GPIO callback */
    gpio_init_callback(&button_cb_data, button_pressed_callback, BIT(button.pin));
    ret = gpio_add_callback(button.port, &button_cb_data);
    if (ret < 0) {
        return ret;
    }
    
    /* Initialize state */
    last_button_state = (gpio_pin_get_dt(&button) == 0);
    
    return 0;
}

