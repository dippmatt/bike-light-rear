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
#ifdef BLUETOOTH
#include "bluetooth.h"
#endif /* BLUETOOTH */
#ifdef DEBUG
#include "i2c_scanner.h"
#endif

/* Global sensor buffer and environmental state */
sensor_buffer_t g_sensor_buffer;
environmental_state_t g_env_state;

static void button_event_handler(enum button_press_type press_type)
{
    ARG_UNUSED(press_type);
    
#ifdef BLUETOOTH
#ifdef BLE_MASTER
    /* In master mode: send button press command to peripheral */
    debug_printk("Button pressed - sending command to peripheral\n");
    ble_toogle_led();
#else
    /* In peripheral mode or no BLE master: handle locally */
    /* Advance state machine on button press */
    main_state_machine_on_button_press();
    enum system_state new_state = main_state_machine_get_state();

    /* If we transitioned to LED_OFF, enter sleep mode */
    if (new_state == LED_OFF) {
        debug_printk("Transitioning to LED_OFF - entering sleep mode\n");
        /* Give a short delay for user feedback (status LED off) */
        k_msleep(100);
        power_management_enter_sleep();
    }
#endif /* BLE_MASTER */
#else
    /* No Bluetooth: handle locally */
    /* Advance state machine on button press */
    main_state_machine_on_button_press();
    enum system_state new_state = main_state_machine_get_state();

    /* If we transitioned to LED_OFF, enter sleep mode */
    if (new_state == LED_OFF) {
        debug_printk("Transitioning to LED_OFF - entering sleep mode\n");
        /* Give a short delay for user feedback (status LED off) */
        k_msleep(100);
        power_management_enter_sleep();
    }
#endif /* BLUETOOTH */
}

int main(void)
{
    int ret;
    
    debug_printk("System starting...\n");
    
    /* Initialize global sensor buffer */
    pthread_rwlock_init(&g_sensor_buffer.lock, NULL);
    g_sensor_buffer.write_index = 0;
    
    /* Initialize environmental state to safe defaults */
    g_env_state.is_braking = false;
    g_env_state.ambient_dark = false;
    g_env_state.previous_brightness = 0;
    
    /* Initialize main state machine (LED control) */
    ret = main_state_machine_init();
    if (ret != 0) {
        /* State machine initialization failed */
        return -1;
    }
    
#ifdef BLUETOOTH
    /* Initialize Bluetooth Low Energy */
    ret = bluetooth_init();
    if (ret != 0) {
        debug_printk("BLE Init failed: %d\n", ret);
        /* Continue anyway - BLE is optional */
    } else {
        debug_printk("BLE initialized successfully\n");
    }
#endif /* BLUETOOTH */
    debug_printk("Statement after BLE initialization\n");
    
    /* Initialize button GPIO (but don't enable interrupts yet) */
    int err = -1;
    err = button_init(button_event_handler);
	if (err) {
		debug_printk("Button Init failed: %d\n", err);
		return err;
	}
	debug_printk("Button GPIO configured (interrupts not yet enabled)...\n");
    
#ifdef DEBUG
    /* Scan I2C bus for debugging */
    const struct device *const i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));
    if (i2c_dev != NULL && device_is_ready(i2c_dev)) {
        debug_printk("Scanning I2C bus during initialization...\n");
        scan_i2c_bus(i2c_dev);
    } else {
        debug_printk("I2C bus not available for scanning\n");
    }
#endif
    
    /* Enable button interrupts AFTER slow initialization tasks
     * This prevents spurious button events during I2C scanning and other init */
    err = button_enable_interrupts();
    if (err) {
        debug_printk("Button interrupt enable failed: %d\n", err);
        return err;
    }
    debug_printk("Button interrupts enabled. Ready for events...\n");
    
    /* Main loop (only reached if NOT in LED_OFF state) */
    while (1) {
        /* Sleep to save power - timers and interrupts will wake the CPU */
        k_msleep(1000);
    }

    return 0;
}
