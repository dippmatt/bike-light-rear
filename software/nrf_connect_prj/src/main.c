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
#include <zephyr/pm/device.h>
#include <zephyr/sys/poweroff.h>

#include "utils.h"
#include "button.h"
#include "main_state_machine.h"
#include "sensor_data_collector.h"
#include "power_management.h"
#ifdef DEBUG
#include "i2c_scanner.h"
#endif

sensor_buffer_t g_sensor_buffer;
environmental_state_t g_env_state;

void power_management_enter_sleep(void)
{
    debug_printk("Entering system off mode (deep sleep)...\n");
    
    int ret = button_configure_wakeup();
    if (ret < 0) {
        debug_printk("Error: Failed to configure button wakeup (%d)\n", ret);
        return;
    }
    
    const struct device *const cons = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
    if (device_is_ready(cons)) {
        int rc = pm_device_action_run(cons, PM_DEVICE_ACTION_SUSPEND);
        if (rc < 0) {
            debug_printk("Warning: Could not suspend console (%d)\n", rc);
        }
    }
    
    k_msleep(50);
    sys_poweroff();
}

static void button_event_handler(enum button_evt evt)
{
    if (evt == BUTTON_EVT_PRESSED) {
        debug_printk("Button pressed\n");
        enum led_brightness prev_state = main_state_machine_get_state();
        main_state_machine_on_button_press();
        enum led_brightness new_state = main_state_machine_get_state();
        
        if (new_state == LED_OFF && prev_state != LED_OFF) {
            debug_printk("Transitioning to LED_OFF - entering sleep\n");
            k_msleep(100);
            power_management_enter_sleep();
        }
    }
}

int main(void)
{
    debug_printk("System starting...\n");
    
    /* Initialize global sensor buffer */
    pthread_rwlock_init(&g_sensor_buffer.lock, NULL);
    g_sensor_buffer.write_index = 0;
    
    /* Initialize environmental state */
    g_env_state.is_braking = false;
    g_env_state.ambient_dark = false;
    g_env_state.previous_brightness = 0;
    
    /* Initialize main state machine */
    int ret = main_state_machine_init();
    if (ret != 0) {
        return -1;
    }
    
    /* Initialize button */
    int err = button_init(button_event_handler);
    if (err) {
        debug_printk("Button init failed: %d\n", err);
        return err;
    }
    debug_printk("Button GPIO configured\n");
    
#ifdef DEBUG
    const struct device *const i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));
    if (i2c_dev && device_is_ready(i2c_dev)) {
        debug_printk("Scanning I2C bus...\n");
        scan_i2c_bus(i2c_dev);
    }
#endif
    
    /* Enable button interrupts after initialization */
    err = button_enable_interrupts();
    if (err) {
        debug_printk("Button interrupt enable failed: %d\n", err);
        return err;
    }
    debug_printk("Button interrupts enabled\n");
    
    /* Check if starting in LED_OFF state */
    if (main_state_machine_get_state() == LED_OFF) {
        debug_printk("Starting in LED_OFF mode - entering deep sleep\n");
        k_msleep(100);
        power_management_enter_sleep();
    }
    
    /* Main loop */
    while (1) {
        k_msleep(1000);
    }

    return 0;
}
