/*
 * Copyright (c) 2016 Intel Corporation
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-LICENSE-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/devicetree.h>
#include <zephyr/init.h>
#include <hal/nrf_gpio.h>
#include "button_control.h"

# define TIMER_INTERVAL_MS 500

/* PWM period for 10kHz = 100 microseconds */
#define PWM_PERIOD_USEC 100
/* 50% duty cycle = 50 microseconds */
#define PWM_DUTY_CYCLE_USEC 50

/* GPIO device specification for STATUS_LED */
static const struct gpio_dt_spec status_led = GPIO_DT_SPEC_GET(STATUS_LED_NODE, gpios);

/* Timer for STATUS_LED blinking */
static struct k_timer led_timer;

/**
 * @brief STATUS_LED timer expiry callback
 * 
 * Simply toggles the LED on every timer tick.
 */
static void led_timer_expiry(struct k_timer *timer)
{
    gpio_pin_toggle_dt(&status_led);
}

int main(void)
{
    int ret;
    
    /* Check if STATUS_LED device is ready */
    if (!gpio_is_ready_dt(&status_led)) {
        return -1;
    }
    /* Configure STATUS_LED as output and initialize to off */
    ret = gpio_pin_configure_dt(&status_led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        return -1;
    }

    /* Initialize button control module */
    ret = button_control_init();
    if (ret != 0) {
        /* Button control initialization failed */
        return -1;
    }
    
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
