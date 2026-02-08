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
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>

#include "light_modes.h"
#include "sensor_data_collector.h"
#include "main_state_machine.h"
#include "utils.h"

/* GPIO Devicetree Specifications */
#define MAIN_LED_PWM_NODE   DT_ALIAS(main_led0)
#define STATUS_LED_NODE     DT_ALIAS(led0)

/* Timer interval for STATUS_LED blinking */
#define STATUS_TIMER_INTERVAL 500

/* PWM period for 1kHz = 1000 microseconds (1ms) */
#define PWM_PERIOD_USEC 1000
/* 50% duty cycle - scaled for 1ms period */
#define PWM_DUTY_CYCLE_30_USEC 300
/* 80% duty cycle - scaled for 1ms period */
#define PWM_DUTY_CYCLE_80_USEC 800
/* 100% duty cycle - scaled for 1ms period */
#define PWM_DUTY_CYCLE_100_USEC 1000

/* Blinking mode timing */
#define BLINK_INTERVAL_MS 1500      /* Flash every 1.5 seconds */
#define BLINK_FLASH_DURATION_MS 70 /* Flash duration: 70ms */

/* PWM device specification from devicetree */
static const struct pwm_dt_spec main_led = PWM_DT_SPEC_GET(MAIN_LED_PWM_NODE);

/* GPIO device specification for STATUS_LED */
static const struct gpio_dt_spec status_led = GPIO_DT_SPEC_GET(STATUS_LED_NODE, gpios);

/* Timer for blinking mode */
static struct k_timer blink_timer;
static struct k_timer flash_timer;
static atomic_t blinking_active = ATOMIC_INIT(0);
static atomic_t flash_step = ATOMIC_INIT(0); /* Tracks flash sequence: 0=after first flash, 1=after gap, 2=after second flash */

/* Work items to defer PWM writes from timer ISR to thread context.
 * k_timer expiry callbacks run in ISR context where driver API calls
 * (like pwm_set_pulse_dt) are not guaranteed to be safe. */
static void flash_work_handler(struct k_work *work);
static void blink_work_handler(struct k_work *work);
static struct k_work flash_work;
static struct k_work blink_work;

/* Timer for STATUS_LED blinking */
static struct k_timer status_led_timer;

/* Current PWM pulse width in microseconds */
static uint32_t current_pwm_usec = 0;

/**
 * @brief Initialize PWM LED hardware
 * 
 * Configures PWM device and sets LED to off state.
 * 
 * @return 0 on success, negative error code on failure
 */
int flash_timers_init(void)
{
    /* Check if PWM device is ready */
    if (!pwm_is_ready_dt(&main_led)) {
        return -ENODEV;
    }
    
    /* Initialize PWM LED: set period and turn off (0% pulse width) */
    int ret = pwm_set_dt(&main_led, PWM_USEC(PWM_PERIOD_USEC), 0);
    if (ret < 0) {
        return ret;
    }
    
    /* Initialize work items for deferred PWM writes from ISR */
    k_work_init(&flash_work, flash_work_handler);
    k_work_init(&blink_work, blink_work_handler);
    
    /* Initialize blinking timers */
    k_timer_init(&blink_timer, blink_timer_expiry, NULL);
    k_timer_init(&flash_timer, flash_timer_expiry, NULL);
    atomic_set(&blinking_active, 0);
    atomic_set(&flash_step, 0);
    
    return 0;
}

/**
 * @brief Initialize Status LED GPIO
 * 
 * Initializes and starts the timer for the STATUS LED.
 * 
 * @return 0 on success, negative error code on failure
 */
int status_led_init(void)
{
    /* Check if STATUS_LED device is ready */
    if (!gpio_is_ready_dt(&status_led)) {
        debug_printk("Error: STATUS_LED device not ready\n");
        return -ENODEV;
    }
    
    /* Configure STATUS_LED as output and initialize to off */
    int ret = gpio_pin_configure_dt(&status_led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        debug_printk("Error configuring STATUS_LED: %d\n", ret);
        return ret;
    }
    /* Initialize status LED timer */
    k_timer_init(&status_led_timer, status_led_timer_expiry, NULL);
    return 0;
}

/**
 * @brief Flash work handler (runs in system workqueue thread context)
 * 
 * Handles the double-flash sequence: 80% (first) -> 50% (gap) -> 80% (second) -> 50% (done)
 * Deferred from flash_timer_expiry ISR via k_work to make PWM driver calls safe.
 */
static void flash_work_handler(struct k_work *work)
{
    ARG_UNUSED(work);
    
    /* Only update if blinking is still active - prevents interference with SMART_MODE */
    if (!atomic_get(&blinking_active)) {
        return;
    }

    atomic_val_t step = atomic_get(&flash_step);
    if (step == 0) {
        /* After first flash (80%): return to 50% for gap */
        pwm_set_pulse_dt(&main_led, PWM_USEC(PWM_DUTY_CYCLE_30_USEC));
        current_pwm_usec = PWM_DUTY_CYCLE_30_USEC;
        debug_printk("PWM duty cycle changed to %u us (flash timer - gap after first flash)\n", current_pwm_usec);
        atomic_inc(&flash_step);
        /* Start flash timer again for second flash */
        k_timer_start(&flash_timer, K_MSEC(BLINK_FLASH_DURATION_MS), K_NO_WAIT);
    } else if (step == 1) {
        /* After gap (50%): flash to 80% for second flash */
        pwm_set_pulse_dt(&main_led, PWM_USEC(PWM_DUTY_CYCLE_80_USEC));
        current_pwm_usec = PWM_DUTY_CYCLE_80_USEC;
        debug_printk("PWM duty cycle changed to %u us (flash timer - second flash)\n", current_pwm_usec);
        atomic_inc(&flash_step);
        /* Start flash timer to return to 50% after second flash */
        k_timer_start(&flash_timer, K_MSEC(BLINK_FLASH_DURATION_MS), K_NO_WAIT);
    } else if (step == 2) {
        /* After second flash (80%): return to 50% and wait for blink timer */
        pwm_set_pulse_dt(&main_led, PWM_USEC(PWM_DUTY_CYCLE_30_USEC));
        current_pwm_usec = PWM_DUTY_CYCLE_30_USEC;
        debug_printk("PWM duty cycle changed to %u us (flash timer - second flash done)\n", current_pwm_usec);
        atomic_set(&flash_step, 0);
        /* Don't start flash timer - wait for blink_timer to start next cycle */
    }
}

/**
 * @brief Flash timer expiry callback (ISR context)
 * 
 * Defers actual PWM work to flash_work_handler via system workqueue.
 */
void flash_timer_expiry(struct k_timer *timer)
{
    ARG_UNUSED(timer);
    k_work_submit(&flash_work);
}

/**
 * @brief Blink work handler (runs in system workqueue thread context)
 * 
 * Triggers the first flash to 80% duty cycle.
 * Deferred from blink_timer_expiry ISR via k_work to make PWM driver calls safe.
 */
static void blink_work_handler(struct k_work *work)
{
    ARG_UNUSED(work);
    
    if (!atomic_get(&blinking_active)) {
        return;
    }

    /* Flash to 80% duty cycle (first flash) */
    pwm_set_pulse_dt(&main_led, PWM_USEC(PWM_DUTY_CYCLE_80_USEC));
    current_pwm_usec = PWM_DUTY_CYCLE_80_USEC;
    debug_printk("PWM duty cycle changed to %u us (blink timer - first flash)\n", current_pwm_usec);
    
    /* Reset flash step to start new double-flash sequence */
    atomic_set(&flash_step, 0);
    
    /* Start flash timer to return to 50% after 70ms */
    k_timer_start(&flash_timer, K_MSEC(BLINK_FLASH_DURATION_MS), K_NO_WAIT);
}

/**
 * @brief Blink timer expiry callback (ISR context)
 * 
 * Defers actual PWM work to blink_work_handler via system workqueue.
 */
void blink_timer_expiry(struct k_timer *timer)
{
    ARG_UNUSED(timer);
    k_work_submit(&blink_work);
}

/**
 * @brief STATUS_LED timer expiry callback
 * 
 * Toggles the LED on every timer tick, but only if not in LED_OFF mode.
 * When in LED_OFF mode, the LED stays off.
 */
void status_led_timer_expiry(struct k_timer *timer)
{
    ARG_UNUSED(timer);
    
    /* Only blink if not in LED_OFF mode */
    if (main_state_machine_get_state() != LED_OFF) {
        gpio_pin_toggle_dt(&status_led);
    } else {
        /* Ensure LED stays off in LED_OFF state */
        gpio_pin_set_dt(&status_led, 0);
    }
}

/**
 * @brief Set LED brightness to 0% (off)
 * Stops any active blinking mode.
 */
void light_modes_stop_blinking(void)
{
    /* Stop blinking if active */
    if (atomic_get(&blinking_active)) {
        atomic_set(&blinking_active, 0);
        k_timer_stop(&blink_timer);
        k_timer_stop(&flash_timer);
        atomic_set(&flash_step, 0);
    }
    
    pwm_set_pulse_dt(&main_led, 0);
    current_pwm_usec = 0;
    debug_printk("PWM duty cycle changed to %u us\n", current_pwm_usec);
}

/**
 * @brief Set LED brightness to 50%
 */
void light_modes_set_50_percent(void)
{    
    pwm_set_pulse_dt(&main_led, PWM_USEC(PWM_DUTY_CYCLE_30_USEC));
    current_pwm_usec = PWM_DUTY_CYCLE_30_USEC;
    debug_printk("PWM duty cycle changed to %u us\n", current_pwm_usec);
}

/**
 * @brief Set LED to high visibility blinking mode
 * 
 * LED runs at 50% duty cycle normally, and flashes to 80% for 100ms every 1.5 seconds.
 */
void light_modes_set_100_percent(void)
{
    /* Set initial state to 50% */
    pwm_set_pulse_dt(&main_led, PWM_USEC(PWM_DUTY_CYCLE_30_USEC));
    current_pwm_usec = PWM_DUTY_CYCLE_30_USEC;
    debug_printk("PWM duty cycle changed to %u us\n", current_pwm_usec);
    
    /* Start blinking mode */
    atomic_set(&blinking_active, 1);
    k_timer_start(&blink_timer, K_MSEC(BLINK_INTERVAL_MS), K_MSEC(BLINK_INTERVAL_MS));
}


/**
 * @brief Initialize SMART_MODE
 * 
 * Sets LED to initial state for SMART_MODE operation.
 * Actual brightness is controlled by environmental conditions.
 */
void light_modes_set_smart_mode(void)
{        
    /* Start with LED off - will be updated based on environmental state */
    pwm_set_pulse_dt(&main_led, 0);
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
    if (main_state_machine_get_state() != LED_SMART_MODE) {
        return;
    }
    
    uint32_t target_pwm;
    
    /* Priority: Braking > Ambient Dark > Off */
    if (g_env_state.is_braking) {
        target_pwm = PWM_DUTY_CYCLE_80_USEC;
    } else if (g_env_state.ambient_dark) {
        target_pwm = PWM_DUTY_CYCLE_30_USEC;
    } else {
        /* Bright daylight, not braking - LED off */
        target_pwm = 0;
    }
    
    /* Only update if changed to avoid unnecessary PWM writes */
    if (target_pwm != current_pwm_usec) {
        pwm_set_pulse_dt(&main_led, PWM_USEC(target_pwm));
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

/**
 * @brief Set main LED to off
 * 
 * Sets LED PWM to 0% duty cycle.
 */
void light_modes_off_main_led(void)
{
    pwm_set_pulse_dt(&main_led, 0);
    current_pwm_usec = 0;
    debug_printk("PWM duty cycle changed to %u us (main LED off)\n", current_pwm_usec);
}

/**
 * @brief Start the status LED timer
 * 
 * Starts the periodic timer that blinks the status LED.
 */
void light_modes_start_status_led(void)
{
    k_timer_start(&status_led_timer, K_MSEC(STATUS_TIMER_INTERVAL), K_MSEC(STATUS_TIMER_INTERVAL));
}

/**
 * @brief Stop the status LED timer
 * 
 * Stops the status LED timer and turns off the LED.
 */
void light_modes_stop_status_led(void)
{
    k_timer_stop(&status_led_timer);
    gpio_pin_set_dt(&status_led, 0);
}

