/*
 * Copyright (c) 2024
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BUTTON_CONTROL_H
#define BUTTON_CONTROL_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>

/* GPIO Devicetree Specifications */
#define STATUS_LED_NODE  DT_ALIAS(led0)
#define MAIN_LED_PWM_NODE    DT_ALIAS(pwm_led0)
#define BUTTON_NODE      DT_ALIAS(sw0)

/* Button debounce time in milliseconds */
#define BUTTON_DEBOUNCE_MS       50

/**
 * @brief Initialize the button control module
 * 
 * Configures GPIO pins, sets up interrupts, and initializes debounce timer.
 * Button press will toggle the main LED.
 * 
 * @return 0 on success, negative error code on failure
 */
int button_control_init(void);

#endif /* BUTTON_CONTROL_H */
