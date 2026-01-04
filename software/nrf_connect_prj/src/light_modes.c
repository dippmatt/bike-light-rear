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

#define MAIN_LED_PWM_NODE DT_ALIAS(pwm_led0)

/* PWM settings (1kHz frequency) */
#define PWM_PERIOD_USEC 1000
#define PWM_DUTY_CYCLE_50_USEC 500
#define PWM_DUTY_CYCLE_80_USEC 800

/* Blinking mode timing */
#define BLINK_INTERVAL_MS 1500
#define BLINK_FLASH_DURATION_MS 100

static const struct pwm_dt_spec pwm_led = PWM_DT_SPEC_GET(MAIN_LED_PWM_NODE);
static struct k_timer blink_timer;
static struct k_timer flash_timer;
static uint32_t current_pwm_usec = 0;

static void flash_timer_expiry(struct k_timer *timer);
static void blink_timer_expiry(struct k_timer *timer);

static void stop_blink_timers(void)
{
    k_timer_stop(&blink_timer);
    k_timer_stop(&flash_timer);
}

int light_modes_init(void)
{
    if (!pwm_is_ready_dt(&pwm_led)) {
        return -ENODEV;
    }
    
    int ret = pwm_set_dt(&pwm_led, PWM_USEC(PWM_PERIOD_USEC), 0);
    if (ret < 0) {
        return ret;
    }
    
    k_timer_init(&blink_timer, blink_timer_expiry, NULL);
    k_timer_init(&flash_timer, flash_timer_expiry, NULL);
    
    return 0;
}

static void flash_timer_expiry(struct k_timer *timer)
{
    ARG_UNUSED(timer);
    pwm_set_pulse_dt(&pwm_led, PWM_USEC(PWM_DUTY_CYCLE_50_USEC));
    current_pwm_usec = PWM_DUTY_CYCLE_50_USEC;
    debug_printk("PWM: Flash end, back to 50%% (%u us)\n", current_pwm_usec);
}

static void blink_timer_expiry(struct k_timer *timer)
{
    ARG_UNUSED(timer);
    pwm_set_pulse_dt(&pwm_led, PWM_USEC(PWM_DUTY_CYCLE_80_USEC));
    current_pwm_usec = PWM_DUTY_CYCLE_80_USEC;
    debug_printk("PWM: Flash to 80%% (%u us)\n", current_pwm_usec);
    k_timer_start(&flash_timer, K_MSEC(BLINK_FLASH_DURATION_MS), K_NO_WAIT);
}

void light_modes_set_off(void)
{
    stop_blink_timers();
    pwm_set_pulse_dt(&pwm_led, 0);
    current_pwm_usec = 0;
    debug_printk("PWM: OFF (0 us)\n");
}

void light_modes_set_50_percent(void)
{
    stop_blink_timers();
    pwm_set_pulse_dt(&pwm_led, PWM_USEC(PWM_DUTY_CYCLE_50_USEC));
    current_pwm_usec = PWM_DUTY_CYCLE_50_USEC;
    debug_printk("PWM: 50%% (%u us)\n", current_pwm_usec);
}

void light_modes_set_50_80_flash(void)
{
    stop_blink_timers();
    pwm_set_pulse_dt(&pwm_led, PWM_USEC(PWM_DUTY_CYCLE_50_USEC));
    current_pwm_usec = PWM_DUTY_CYCLE_50_USEC;
    debug_printk("PWM: 50-80 flash mode (%u us base)\n", current_pwm_usec);
    k_timer_start(&blink_timer, K_MSEC(BLINK_INTERVAL_MS), K_MSEC(BLINK_INTERVAL_MS));
}

void light_modes_set_smart_mode(void)
{
    stop_blink_timers();
    pwm_set_pulse_dt(&pwm_led, 0);
    current_pwm_usec = 0;
    debug_printk("PWM: SMART_MODE init (0 us)\n");
    light_modes_update_smart_pwm();
}

void light_modes_update_smart_pwm(void)
{
    uint32_t target_pwm;
    
    if (g_env_state.is_braking) {
        target_pwm = PWM_DUTY_CYCLE_80_USEC;
    } else if (g_env_state.ambient_dark) {
        target_pwm = PWM_DUTY_CYCLE_50_USEC;
    } else {
        target_pwm = 0;
    }
    
    if (target_pwm != current_pwm_usec) {
        pwm_set_pulse_dt(&pwm_led, PWM_USEC(target_pwm));
        current_pwm_usec = target_pwm;
        debug_printk("PWM: SMART_MODE update (%u us)\n", current_pwm_usec);
    }
}

uint32_t light_modes_get_current_pwm(void)
{
    return current_pwm_usec;
}
