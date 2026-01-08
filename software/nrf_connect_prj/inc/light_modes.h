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

#ifndef LIGHT_MODES_H
#define LIGHT_MODES_H

#include <stdint.h>
#include <stdbool.h>
#include <zephyr/kernel.h>

/**
 * @brief LED state machine states
 */
enum system_state {
    LED_OFF = 0,     /* 0% duty cycle */
    LED_50_PERCENT,  /* 50% duty cycle */
    LED_50_80_FLASH, /* High visibility mode: 50% with periodic 80% flashes */
    LED_SMART_MODE   /* Smart mode: adaptive brightness based on braking and ambient light */
};

/** @brief Timer expiry callback for the flash timer */
void flash_timer_expiry(struct k_timer *timer);

/** @brief Timer expiry callback for the blink timer */
void blink_timer_expiry(struct k_timer *timer);

/** @brief Timer expiry callback for the status LED timer */
void status_led_timer_expiry(struct k_timer *timer);

/**
 * @brief Initialize PWM LED hardware
 * 
 * Configures PWM device and sets LED to off state.
 * 
 * @return 0 on success, negative error code on failure
 */
int flash_timers_init(void);

/**
 * @brief Initialize Status LED GPIO
 * 
 * Initializes and starts the timer for the STATUS LED.
 * 
 * @return 0 on success, negative error code on failure
 */
int status_led_init(void);

/**
 * @brief Stop any active blinking mode
 * Sets LED brightness to 0% (off).
 */
void light_modes_stop_blinking(void);

/**
 * @brief Set LED brightness to 50%
 * Stops any active blinking mode.
 */
void light_modes_set_50_percent(void);

/**
 * @brief Set LED to high visibility blinking mode
 * 
 * LED runs at 50% duty cycle normally, and flashes to 80% for 100ms every second.
 */
void light_modes_set_100_percent(void);

/**
 * @brief Initialize SMART_MODE
 * 
 * Sets LED to initial state for SMART_MODE operation.
 * Actual brightness is controlled by environmental conditions.
 */
void light_modes_set_smart_mode(void);

/**
 * @brief Update PWM in SMART_MODE based on environmental state
 * 
 * Should be called when environmental conditions change (braking, ambient light).
 * Reads global environmental state and adjusts PWM accordingly.
 */
void light_modes_update_smart_pwm(void);

/**
 * @brief Get current PWM pulse width
 * 
 * @return Current PWM pulse width in microseconds
 */
uint32_t light_modes_get_current_pwm(void);

/**
 * @brief Start the status LED timer
 * 
 * Starts the periodic timer that blinks the status LED.
 */
void light_modes_start_status_led(void);

/**
 * @brief Stop the status LED timer
 * 
 * Stops the status LED timer and turns off the LED.
 */
void light_modes_stop_status_led(void);

#endif /* LIGHT_MODES_H */

