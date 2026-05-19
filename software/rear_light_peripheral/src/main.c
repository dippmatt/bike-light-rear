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
#include <zephyr/drivers/hwinfo.h>
#include <hal/nrf_power.h>

#include "utils.h"
#include "button.h"
#include "main_state_machine.h"
#include "light_modes.h"
#include "sensor_data_collector.h"
#include "power_management.h"
#include "ble_peripheral.h"
#ifdef DEBUG
#include "i2c_scanner.h"
#endif

/* Global sensor buffer and environmental state */
sensor_buffer_t g_sensor_buffer;
environmental_state_t g_env_state;

/* Delayed work item for entering sleep mode.
 * Avoids blocking the system workqueue with k_msleep() in button_event_handler. */
static void sleep_work_handler(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(sleep_work, sleep_work_handler);

/* USB VBUS monitoring: track state for disconnect detection in IDLE_CHARGING */
static bool usb_vbus_connected_prev = false;
static void usb_monitor_work_handler(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(usb_monitor_work, usb_monitor_work_handler);

/**
 * @brief Read current USB VBUS connection state from nRF52 POWER peripheral
 */
static bool check_usb_vbus_state(void)
{
    uint32_t usbregstatus = NRF_POWER->USBREGSTATUS;
    return (usbregstatus & POWER_USBREGSTATUS_VBUSDETECT_Msk) != 0;
}

/**
 * @brief Log reset reason for debugging
 *
 * Reads and logs the reset cause flags, then clears them.
 * Should be called immediately at boot to capture reset reason.
 * @param cause_out If non-NULL, reset cause is stored here before clearing
 */
static void log_reset_reason(uint32_t *cause_out)
{
    uint32_t cause = 0;
    int ret = hwinfo_get_reset_cause(&cause);

    if (cause_out != NULL) {
        *cause_out = cause;
    }

    if (ret != 0) {
        debug_printk("Failed to read reset cause: %d\n", ret);
        return;
    }

    if (cause == 0) {
        debug_printk("Reset reason: Unknown (no flags set)\n");
        return;
    }

    debug_printk("Reset reason flags: 0x%08x\n", cause);

    if (cause & RESET_POR) {
        debug_printk("  - POR (Power-On Reset) - likely USB connect or power cycle\n");
    }
    if (cause & RESET_BROWNOUT) {
        debug_printk("  - BROWNOUT (Brownout Reset) - likely USB disconnect voltage drop\n");
    }
    if (cause & RESET_SOFTWARE) {
        debug_printk("  - SOFTWARE (Software Reset)\n");
    }
    if (cause & RESET_PIN) {
        debug_printk("  - PIN (External Reset Pin)\n");
    }
    if (cause & RESET_WATCHDOG) {
        debug_printk("  - WATCHDOG (Watchdog Timer Reset)\n");
    }
    if (cause & RESET_LOW_POWER_WAKE) {
        debug_printk("  - LOW_POWER_WAKE (System OFF wake - button interrupt)\n");
    }

    /* Clear reset cause flags after reading */
    hwinfo_clear_reset_cause();
}

/**
 * @brief Log USB VBUS connection state for debugging
 * 
 * Reads USB VBUS status from nRF52 POWER peripheral registers
 * to detect if USB cable is connected.
 */
static void log_usb_vbus_state(void)
{
    /* Read USBREGSTATUS register to check VBUS state */
    uint32_t usbregstatus = NRF_POWER->USBREGSTATUS;
    
    /* Check VBUSDETECT bit (bit 0) */
    if (usbregstatus & POWER_USBREGSTATUS_VBUSDETECT_Msk) {
        debug_printk("USB VBUS: Connected (detected)\n");
    } else {
        debug_printk("USB VBUS: Not connected\n");
    }
    
    /* Also log full register value for debugging */
    debug_printk("USBREGSTATUS register: 0x%08x\n", usbregstatus);
}

static void sleep_work_handler(struct k_work *work)
{
    ARG_UNUSED(work);
    power_management_enter_sleep();
}

/**
 * @brief Periodic USB VBUS check: when in IDLE_CHARGING and USB disconnects,
 * transition to LED_OFF and schedule sleep.
 */
static void usb_monitor_work_handler(struct k_work *work)
{
    ARG_UNUSED(work);

    bool connected = check_usb_vbus_state();

    if (main_state_machine_get_state() == IDLE_CHARGING &&
        usb_vbus_connected_prev && !connected) {
        debug_printk("USB disconnected in IDLE_CHARGING - transitioning to LED_OFF then sleep\n");
        main_state_machine_set_state(LED_OFF);
        k_work_schedule(&sleep_work, K_MSEC(100));
    }

    usb_vbus_connected_prev = connected;
    main_state_machine_set_usb_connected(connected);
    light_modes_set_status_led_charging(connected);
    k_work_reschedule(&usb_monitor_work, K_SECONDS(1));
}

static void button_event_handler(enum button_press_type press_type)
{
    ARG_UNUSED(press_type);
    
    /* Advance state machine on button press */
    main_state_machine_on_button_press();
    enum system_state new_state = main_state_machine_get_state();

    /* If we transitioned to LED_OFF, schedule sleep after 100ms delay.
     * Using k_work_delayable avoids blocking the system workqueue. */
    if (new_state == LED_OFF) {
        debug_printk("Transitioning to LED_OFF - scheduling sleep mode\n");
        k_work_schedule(&sleep_work, K_MSEC(100));
    }
}

int main(void)
{
    int ret;
    uint32_t boot_reset_cause = 0;
    bool boot_usb_connected;

    debug_printk("System starting...\n");

    /* Capture reset reason and USB state immediately at boot (before clearing) */
    log_reset_reason(&boot_reset_cause);
    boot_usb_connected = check_usb_vbus_state();
    log_usb_vbus_state();

    /* Initialize global sensor buffer */
    pthread_rwlock_init(&g_sensor_buffer.lock, NULL);
    g_sensor_buffer.write_index = 0;
    
    /* Initialize environmental state to safe defaults */
    g_env_state.is_braking = false;
    g_env_state.ambient_dark = false;
    g_env_state.previous_brightness = 0;
    g_env_state.low_battery = false;
    
    /* Initialize main state machine (LED control) */
    ret = main_state_machine_init();
    if (ret != 0) {
        /* State machine initialization failed */
        return -1;
    }

    /* If boot was due to USB POR with USB connected, start in IDLE_CHARGING */
    if ((boot_reset_cause & RESET_POR) && boot_usb_connected) {
        debug_printk("USB POR with VBUS connected - entering IDLE_CHARGING\n");
        main_state_machine_set_state(IDLE_CHARGING);
    }
    usb_vbus_connected_prev = check_usb_vbus_state();
    main_state_machine_set_usb_connected(usb_vbus_connected_prev);
    light_modes_set_status_led_charging(boot_usb_connected);

    /* Status LED off until battery monitor sets low_battery state */
    light_modes_set_status_led_battery(false);

    /* Initialize Bluetooth Low Energy */
    ret = bluetooth_init();
    if (ret != 0) {
        debug_printk("BLE Init failed: %d\n", ret);
        /* Continue anyway - BLE is optional */
    } else {
        debug_printk("BLE initialized successfully\n");
    }
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

    /* Start USB VBUS monitoring (for IDLE_CHARGING -> LED_OFF on disconnect) */
    k_work_schedule(&usb_monitor_work, K_SECONDS(1));

    /* Main loop (only reached if NOT in LED_OFF state) */
    while (1) {
        /* Sleep to save power - timers and interrupts will wake the CPU */
        k_msleep(1000);
    }

    return 0;
}
