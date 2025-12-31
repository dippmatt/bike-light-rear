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

#include <zephyr/drivers/pwm.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>

#include "light_modes.h"
#include "sensor_data_collector.h"
#include "utils.h"

/* GPIO Devicetree Specifications */
#define MAIN_LED_PWM_NODE   DT_ALIAS(pwm_led0)

/* PWM period for 1kHz = 1000 microseconds (1ms) */
#define PWM_PERIOD_USEC 1000
/* 50% duty cycle - scaled for 1ms period */
#define PWM_DUTY_CYCLE_50_USEC 500
/* 80% duty cycle - scaled for 1ms period */
#define PWM_DUTY_CYCLE_80_USEC 800
/* 100% duty cycle - scaled for 1ms period */
#define PWM_DUTY_CYCLE_100_USEC 1000

/* Blinking mode timing */
#define BLINK_INTERVAL_MS 1500      /* Flash every 1.5 seconds */
#define BLINK_FLASH_DURATION_MS 100 /* Flash duration: 100ms */

/* PWM device specification from devicetree */
static const struct pwm_dt_spec pwm_led = PWM_DT_SPEC_GET(MAIN_LED_PWM_NODE);

/* Timer for blinking mode */
static struct k_timer blink_timer;
static struct k_timer flash_timer;
static bool blinking_active = false;

/* Current PWM pulse width in microseconds */
static uint32_t current_pwm_usec = 0;

/* Forward declarations */
static void flash_timer_expiry(struct k_timer *timer);
static void blink_timer_expiry(struct k_timer *timer);

/**
 * @brief Initialize PWM LED hardware
 * 
 * Configures PWM device and sets LED to off state.
 * 
 * @return 0 on success, negative error code on failure
 */
int light_modes_init(void)
{
    /* Check if PWM device is ready */
    if (!pwm_is_ready_dt(&pwm_led)) {
        return -ENODEV;
    }
    
    /* Initialize PWM LED: set period and turn off (0% pulse width) */
    int ret = pwm_set_dt(&pwm_led, PWM_USEC(PWM_PERIOD_USEC), 0);
    if (ret < 0) {
        return ret;
    }
    
    /* Initialize blinking timers */
    k_timer_init(&blink_timer, blink_timer_expiry, NULL);
    k_timer_init(&flash_timer, flash_timer_expiry, NULL);
    blinking_active = false;
    
    return 0;
}

/**
 * @brief Flash timer expiry callback
 * 
 * Returns LED to 50% duty cycle after flash period.
 * Only acts if blinking mode is still active.
 */
static void flash_timer_expiry(struct k_timer *timer)
{
    (void)timer; /* Parameter required by Zephyr timer API but not used */
    
    /* Only update if blinking is still active - prevents interference with SMART_MODE */
    if (blinking_active) {
        /* Return to 50% duty cycle */
        pwm_set_pulse_dt(&pwm_led, PWM_USEC(PWM_DUTY_CYCLE_50_USEC));
        current_pwm_usec = PWM_DUTY_CYCLE_50_USEC;
        debug_printk("PWM duty cycle changed to %u us (flash timer)\n", current_pwm_usec);
    }
}

/**
 * @brief Blink timer expiry callback
 * 
 * Triggers a flash to 80% duty cycle every 1.5 seconds.
 */
static void blink_timer_expiry(struct k_timer *timer)
{
    (void)timer; /* Parameter required by Zephyr timer API but not used */
    
    if (blinking_active) {
        /* Flash to 80% duty cycle */
        pwm_set_pulse_dt(&pwm_led, PWM_USEC(PWM_DUTY_CYCLE_80_USEC));
        current_pwm_usec = PWM_DUTY_CYCLE_80_USEC;
        debug_printk("PWM duty cycle changed to %u us\n", current_pwm_usec);
        
        /* Start flash timer to return to 50% after 100ms */
        k_timer_start(&flash_timer, K_MSEC(BLINK_FLASH_DURATION_MS), K_NO_WAIT);
    }
}

/**
 * @brief Set LED brightness to 0% (off)
 * Stops any active blinking mode.
 */
void light_modes_set_off(void)
{
    /* Stop blinking if active */
    if (blinking_active) {
        k_timer_stop(&blink_timer);
        k_timer_stop(&flash_timer);
        blinking_active = false;
    }
    
    pwm_set_pulse_dt(&pwm_led, 0);
    current_pwm_usec = 0;
    debug_printk("PWM duty cycle changed to %u us\n", current_pwm_usec);
}

/**
 * @brief Set LED brightness to 50%
 * Stops any active blinking mode.
 */
void light_modes_set_50_percent(void)
{
    /* Stop blinking if active */
    if (blinking_active) {
        k_timer_stop(&blink_timer);
        k_timer_stop(&flash_timer);
        blinking_active = false;
    }
    
    pwm_set_pulse_dt(&pwm_led, PWM_USEC(PWM_DUTY_CYCLE_50_USEC));
    current_pwm_usec = PWM_DUTY_CYCLE_50_USEC;
    debug_printk("PWM duty cycle changed to %u us\n", current_pwm_usec);
}

/**
 * @brief Set LED to high visibility blinking mode
 * 
 * LED runs at 50% duty cycle normally, and flashes to 80% for 100ms every 1.5 seconds.
 */
void light_modes_set_100_percent(void)
{
    /* Stop any previous blinking */
    if (blinking_active) {
        k_timer_stop(&blink_timer);
        k_timer_stop(&flash_timer);
    }
    
    /* Set initial state to 50% */
    pwm_set_pulse_dt(&pwm_led, PWM_USEC(PWM_DUTY_CYCLE_50_USEC));
    current_pwm_usec = PWM_DUTY_CYCLE_50_USEC;
    debug_printk("PWM duty cycle changed to %u us\n", current_pwm_usec);
    
    /* Start blinking mode */
    blinking_active = true;
    k_timer_start(&blink_timer, K_MSEC(BLINK_INTERVAL_MS), K_MSEC(BLINK_INTERVAL_MS));
}

/**
 * @brief Set LED brightness based on enum value
 * 
 * @param brightness Brightness level to set
 */
void light_modes_set_brightness(enum led_brightness brightness)
{
    switch (brightness) {
        case LED_OFF:
            light_modes_set_off();
            break;
        case LED_50_PERCENT:
            light_modes_set_50_percent();
            break;
        case LED_50_80_FLASH:
            light_modes_set_100_percent();
            break;
        case LED_SMART_MODE:
            light_modes_set_smart_mode();
            break;
    }
}

/**
 * @brief Initialize SMART_MODE
 * 
 * Sets LED to initial state for SMART_MODE operation.
 * Actual brightness is controlled by environmental conditions.
 */
void light_modes_set_smart_mode(void)
{
    /* Stop blinking if active - ensure timers are fully stopped */
    if (blinking_active) {
        k_timer_stop(&blink_timer);
        k_timer_stop(&flash_timer);
        blinking_active = false;
    }
    
    /* Ensure timers are stopped even if blinking_active was already false */
    /* This prevents any lingering timer callbacks from interfering */
    k_timer_stop(&blink_timer);
    k_timer_stop(&flash_timer);
    
    /* Start with LED off - will be updated based on environmental state */
    pwm_set_pulse_dt(&pwm_led, 0);
    current_pwm_usec = 0;
    debug_printk("PWM duty cycle changed to %u us (SMART_MODE init)\n", current_pwm_usec);
    
    /* Initial update based on current environmental state */
    light_modes_update_smart_pwm();
}

/**
 * @brief Update PWM in SMART_MODE based on environmental state
 * 
 * Should be called when environmental conditions change (braking, ambient light).
 * Reads global environmental state and adjusts PWM accordingly.
 * 
 * IMPORTANT: This function should only be called when in SMART_MODE to avoid
 * interference with other modes (like blinking mode timers).
 */
void light_modes_update_smart_pwm(void)
{
    /* Only update if we're actually in SMART_MODE - prevents interference */
    if (blinking_active) {
        /* Blinking mode is active, don't interfere */
        return;
    }
    
    uint32_t target_pwm;
    
    /* Priority: Braking > Ambient Dark > Off */
    if (g_env_state.is_braking) {
        target_pwm = PWM_DUTY_CYCLE_80_USEC;
    } else if (g_env_state.ambient_dark) {
        target_pwm = PWM_DUTY_CYCLE_50_USEC;
    } else {
        /* Bright daylight, not braking - LED off */
        target_pwm = 0;
    }
    
    /* Only update if changed to avoid unnecessary PWM writes */
    if (target_pwm != current_pwm_usec) {
        pwm_set_pulse_dt(&pwm_led, PWM_USEC(target_pwm));
        current_pwm_usec = target_pwm;
        debug_printk("PWM duty cycle changed to %u us (SMART_MODE update)\n", current_pwm_usec);
    }
}

/**
 * @brief Get current PWM pulse width
 * 
 * @return Current PWM pulse width in microseconds
 */
uint32_t light_modes_get_current_pwm(void)
{
    return current_pwm_usec;
}

