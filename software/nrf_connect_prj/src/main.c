/*
 * Copyright (c) 2016 Intel Corporation
 * Copyright (c) 2020 Nordic Semiconductor ASA
 * Copyright (c) 2024
 *
 * SPDX-LICENSE-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>

#include "utils.h"
#include "button.h"
#include "main_state_machine.h"
#ifdef DEBUG
#include "i2c_scanner.h"
#endif

/* GPIO Devicetree Specifications */
#define STATUS_LED_NODE     DT_ALIAS(led0)

/* Timer interval for STATUS_LED blinking */
#define TIMER_INTERVAL_MS 500

/* GPIO device specification for STATUS_LED */
static const struct gpio_dt_spec status_led = GPIO_DT_SPEC_GET(STATUS_LED_NODE, gpios);

/* Timer for STATUS_LED blinking */
static struct k_timer led_timer;

/**
 * @brief STATUS_LED timer expiry callback
 * 
 * Simply toggles the LED on every timer tick.
 */
static void led_timer_expiry()
{
    gpio_pin_toggle_dt(&status_led);
}

int main(void)
{
    int ret;
    
    debug_printk("System starting...\n");
    
    /* Check if STATUS_LED device is ready */
    if (!gpio_is_ready_dt(&status_led)) {
        return -1;
    }
    /* Configure STATUS_LED as output and initialize to off */
    ret = gpio_pin_configure_dt(&status_led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        return -1;
    }

    /* Initialize main state machine (LED control) */
    ret = main_state_machine_init();
    if (ret != 0) {
        /* State machine initialization failed */
        return -1;
    }
    
    /* Initialize button with callback to state machine */
    ret = button_init(main_state_machine_on_button_press);
    if (ret != 0) {
        /* Button initialization failed */
        return -1;
    }
    
#ifdef DEBUG
    /* Scan I2C bus for debugging */
    const struct device *const i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));
    if (i2c_dev != NULL && device_is_ready(i2c_dev)) {
        debug_printk("Scanning I2C bus during initialization...\n");
        scan_i2c_bus(i2c_dev);
    } else {
        debug_printk("I2C bus not available for scanning\n");
    }
#endif
    
    /* Initialize and start LED blink timer */
    /* Timer fires every 500ms, toggling the LED for 1Hz blink (on 500ms, off 500ms) */
    k_timer_init(&led_timer, led_timer_expiry, NULL);
    k_timer_start(&led_timer, K_MSEC(TIMER_INTERVAL_MS), K_MSEC(TIMER_INTERVAL_MS));
    
    /* Main loop */
    while (1) {
        /* Sleep to save power - timers and interrupts will wake the CPU */
        k_msleep(1000);
    }

    return 0;
}
