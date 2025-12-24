/*
 * Copyright (c) 2024
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "button_control.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>

/* PWM period for 10kHz = 100 microseconds */
#define PWM_PERIOD_USEC 100
/* 50% duty cycle = 50 microseconds */
#define PWM_DUTY_CYCLE_USEC 50
/* 100% duty cycle = 100 microseconds (same as period) */
#define PWM_DUTY_CYCLE_100_USEC 100

/* LED brightness states */
enum led_brightness {
    LED_OFF = 0,      /* 0% duty cycle */
    LED_50_PERCENT,  /* 50% duty cycle */
    LED_100_PERCENT  /* 100% duty cycle */
};

/* PWM device specification from devicetree */
static const struct pwm_dt_spec pwm_led = PWM_DT_SPEC_GET(MAIN_LED_PWM_NODE);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

/* LED state - cycles through OFF -> 50% -> 100% -> OFF */
static volatile enum led_brightness led_state = LED_OFF;

/* Debounce timer */
static struct k_timer debounce_timer;

/* Last button state */
static volatile bool last_button_state = false;

/**
 * @brief Debounce timer expiry callback
 * 
 * Checks button state after debounce delay and cycles MAIN_LED brightness on press.
 * Cycles through: OFF (0%) -> 50% -> 100% -> OFF
 */
static void debounce_timer_expiry(struct k_timer *timer)
{
    /* Read current button state (active low: 0 = pressed, 1 = released) */
    int pin_state = gpio_pin_get_dt(&button);
    bool currently_pressed = (pin_state == 0);
    
    /* Cycle MAIN_LED brightness on button press (transition from released to pressed) */
    if (currently_pressed && !last_button_state) {
        switch (led_state) {
            case LED_OFF:
                /* Turn LED on with 10kHz PWM at 50% duty cycle */
                pwm_set_dt(&pwm_led, PWM_USEC(PWM_PERIOD_USEC), PWM_USEC(PWM_DUTY_CYCLE_USEC));
                led_state = LED_50_PERCENT;
                break;
            case LED_50_PERCENT:
                /* Turn LED on with 10kHz PWM at 100% duty cycle */
                pwm_set_dt(&pwm_led, PWM_USEC(PWM_PERIOD_USEC), PWM_USEC(PWM_DUTY_CYCLE_100_USEC));
                led_state = LED_100_PERCENT;
                break;
            case LED_100_PERCENT:
                /* Turn LED off by setting duty cycle to 0 */
                pwm_set_dt(&pwm_led, PWM_USEC(PWM_PERIOD_USEC), 0);
                led_state = LED_OFF;
                break;
        }
    }
    
    last_button_state = currently_pressed;
}

/**
 * @brief GPIO interrupt callback for button
 * 
 * Triggered when button state changes. Starts debounce timer.
 */
static void button_pressed_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    /* Start debounce timer */
    k_timer_start(&debounce_timer, K_MSEC(BUTTON_DEBOUNCE_MS), K_NO_WAIT);
}

/* GPIO callback structure */
static struct gpio_callback button_cb_data;

int button_control_init(void)
{
    int ret;
    
    /* Check if PWM device is ready */
    if (!pwm_is_ready_dt(&pwm_led)) {
        return -ENODEV;
    }
    
    if (!gpio_is_ready_dt(&button)) {
        return -ENODEV;
    }
    
    /* Initialize PWM LED to off state */
    ret = pwm_set_dt(&pwm_led, PWM_USEC(PWM_PERIOD_USEC), 0);
    if (ret < 0) {
        return ret;
    }
    led_state = LED_OFF;
    
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
