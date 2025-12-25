/*
 * Copyright (c) 2016 Intel Corporation
 * Copyright (c) 2020 Nordic Semiconductor ASA
 * Copyright (c) 2024
 *
 * SPDX-LICENSE-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/devicetree.h>
#include <zephyr/init.h>

/* GPIO Devicetree Specifications */
#define STATUS_LED_NODE     DT_ALIAS(led0)
#define MAIN_LED_PWM_NODE   DT_ALIAS(pwm_led0)
#define BUTTON_NODE         DT_ALIAS(sw0)

/* Timer interval for STATUS_LED blinking */
#define TIMER_INTERVAL_MS 500

/* Button debounce time in milliseconds */
#define BUTTON_DEBOUNCE_MS 50

/* PWM period for 10kHz = 100 microseconds */
#define PWM_PERIOD_USEC 100
/* 50% duty cycle = 50 microseconds */
#define PWM_DUTY_CYCLE_USEC 50
/* 100% duty cycle = 100 microseconds (same as period) */
#define PWM_DUTY_CYCLE_100_USEC 100

#define SENSOR_THREAD_PRIORITY 7
#define SENSOR_THREAD_STACK_SIZE 1024

//K_THREAD_DEFINE(sensor_data_collector_id, SENSOR_THREAD_STACK_SIZE, sensor_data_collector, NULL, NULL, NULL, SENSOR_THREAD_PRIORITY, 0, 1000);

/* LED brightness states */
enum led_brightness {
    LED_OFF = 0,      /* 0% duty cycle */
    LED_50_PERCENT,  /* 50% duty cycle */
    LED_100_PERCENT  /* 100% duty cycle */
};

/* GPIO device specification for STATUS_LED */
static const struct gpio_dt_spec status_led = GPIO_DT_SPEC_GET(STATUS_LED_NODE, gpios);

/* PWM device specification from devicetree */
static const struct pwm_dt_spec pwm_led = PWM_DT_SPEC_GET(MAIN_LED_PWM_NODE);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

/* LED state - cycles through OFF -> 50% -> 100% -> OFF */
static volatile enum led_brightness led_state = LED_OFF;

/* Timer for STATUS_LED blinking */
static struct k_timer led_timer;

/* Debounce timer */
static struct k_timer debounce_timer;

/* Last button state */
static volatile bool last_button_state = false;

/* GPIO callback structure */
static struct gpio_callback button_cb_data;

/**
 * @brief Debounce timer expiry callback
 * 
 * Checks button state after debounce delay and cycles MAIN_LED brightness on press.
 * Cycles through: OFF (0%) -> 50% -> 100% -> OFF
 */
static void debounce_timer_expiry()
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
static void button_pressed_callback()
{
    /* Start debounce timer */
    k_timer_start(&debounce_timer, K_MSEC(BUTTON_DEBOUNCE_MS), K_NO_WAIT);
}

/**
 * @brief STATUS_LED timer expiry callback
 * 
 * Simply toggles the LED on every timer tick.
 */
static void led_timer_expiry()
{
    gpio_pin_toggle_dt(&status_led);
}

/**
 * @brief Initialize PWM LED and button control
 * 
 * Configures GPIO pins, sets up interrupts, and initializes debounce timer.
 * Button press will cycle the main LED brightness.
 * 
 * @return 0 on success, negative error code on failure
 */
static int init_pwm_led_and_button(void)
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

    /* Initialize PWM LED and button control */
    ret = init_pwm_led_and_button();
    if (ret != 0) {
        /* PWM LED and button initialization failed */
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
