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

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>

#include "main_state_machine.h"
#include "light_modes.h"
#include "utils.h"
#include "sensor_data_collector.h"

enum led_brightness g_led_state = LED_50_PERCENT;

/* Status LED for SMART_MODE */
#define STATUS_LED_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec status_led = GPIO_DT_SPEC_GET(STATUS_LED_NODE, gpios);
static struct k_timer status_led_timer;

/* Forward declarations for state functions */
static void state_led_50_percent_init(void);
static void state_led_50_percent_terminate(void);
static void state_led_50_80_flash_init(void);
static void state_led_50_80_flash_terminate(void);
static void state_led_smart_mode_init(void);
static void state_led_smart_mode_terminate(void);
static void state_led_off_init(void);
static void state_led_off_terminate(void);

/* State descriptors */
static const state_descriptor_t state_descriptors[] = {
    {LED_50_PERCENT, state_led_50_percent_init, state_led_50_percent_terminate, LED_50_80_FLASH},
    {LED_50_80_FLASH, state_led_50_80_flash_init, state_led_50_80_flash_terminate, LED_SMART_MODE},
    {LED_SMART_MODE, state_led_smart_mode_init, state_led_smart_mode_terminate, LED_OFF},
    {LED_OFF, state_led_off_init, state_led_off_terminate, LED_50_PERCENT},
};

static const state_descriptor_t* get_state_descriptor(enum led_brightness state)
{
    for (size_t i = 0; i < ARRAY_SIZE(state_descriptors); i++) {
        if (state_descriptors[i].state_id == state) {
            return &state_descriptors[i];
        }
    }
    return NULL;
}

static void status_led_timer_expiry(struct k_timer *timer)
{
    ARG_UNUSED(timer);
    gpio_pin_toggle_dt(&status_led);
}

/* State: LED_50_PERCENT */
static void state_led_50_percent_init(void)
{
    debug_printk("State: LED_50_PERCENT init\n");
    light_modes_set_50_percent();
}

static void state_led_50_percent_terminate(void)
{
    debug_printk("State: LED_50_PERCENT terminate\n");
}

/* State: LED_50_80_FLASH */
static void state_led_50_80_flash_init(void)
{
    debug_printk("State: LED_50_80_FLASH init\n");
    light_modes_set_50_80_flash();
}

static void state_led_50_80_flash_terminate(void)
{
    debug_printk("State: LED_50_80_FLASH terminate\n");
}

/* State: LED_SMART_MODE */
static void state_led_smart_mode_init(void)
{
    debug_printk("State: LED_SMART_MODE init\n");
    light_modes_set_smart_mode();
    sensor_threads_start();
    k_timer_start(&status_led_timer, K_MSEC(500), K_MSEC(500));
}

static void state_led_smart_mode_terminate(void)
{
    debug_printk("State: LED_SMART_MODE terminate\n");
    sensor_threads_stop();
    k_timer_stop(&status_led_timer);
    gpio_pin_set_dt(&status_led, 0);
}

/* State: LED_OFF */
static void state_led_off_init(void)
{
    debug_printk("State: LED_OFF init\n");
    light_modes_set_off();
}

static void state_led_off_terminate(void)
{
    debug_printk("State: LED_OFF terminate\n");
}

static void state_transition(enum led_brightness new_state)
{
    const state_descriptor_t *current = get_state_descriptor(g_led_state);
    const state_descriptor_t *next = get_state_descriptor(new_state);
    
    if (!next) {
        debug_printk("Error: Invalid state transition to %d\n", new_state);
        return;
    }
    
    /* Terminate current state */
    if (current && current->terminate) {
        current->terminate();
    }
    
    /* Update state */
    g_led_state = new_state;
    
    /* Initialize new state */
    if (next->init) {
        next->init();
    }
}

int main_state_machine_init(void)
{
    int ret = light_modes_init();
    if (ret != 0) {
        return ret;
    }
    
    /* Initialize status LED GPIO */
    if (!gpio_is_ready_dt(&status_led)) {
        return -1;
    }
    ret = gpio_pin_configure_dt(&status_led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        return -1;
    }
    
    /* Initialize status LED timer */
    k_timer_init(&status_led_timer, status_led_timer_expiry, NULL);
    
    /* Initialize to LED_50_PERCENT state */
    g_led_state = LED_OFF;  /* Set to OFF temporarily so transition works */
    state_transition(LED_50_PERCENT);
    
    return 0;
}

void main_state_machine_on_button_press(void)
{
    const state_descriptor_t *current = get_state_descriptor(g_led_state);
    if (current) {
        debug_printk("State machine: Button press, transitioning from %d to %d\n", 
                     g_led_state, current->next_state);
        state_transition(current->next_state);
    }
}

void main_state_machine_auto_off(bool enter_sleep)
{
    ARG_UNUSED(enter_sleep);
    
    if (g_led_state == LED_SMART_MODE) {
        debug_printk("State machine: Auto-off triggered\n");
        state_transition(LED_OFF);
    }
}

enum led_brightness main_state_machine_get_state(void)
{
    return g_led_state;
}
