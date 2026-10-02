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
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/hwinfo.h>
#include <zephyr/sys/reboot.h>

#include "app_events.h"
#include "state_machine.h"
#include "light.h"
#include "status_led.h"
#include "button.h"
#include "sensors.h"
#include "battery.h"
#include "ble.h"
#include "power.h"
#include "debug.h"
#ifdef DEBUG
#include "i2c_scanner.h"
#endif

/* Time to let the last debug message drain before a software reset */
#define RESET_FLUSH_DELAY_MS 100

/**
 * @brief Recover from a failed essential initialization by resetting
 *
 * Returning from main() would leave the device dead (no light, no button).
 * A software reset gives a transient fault another chance, and the boot
 * policy brings the light back on afterwards (fail operational).
 */
static FUNC_NORETURN void fatal_reset(const char *what, int err)
{
    debug_printk("%s failed: %d - resetting\n", what, err);

    /* Give the debug output time to drain before the reset */
    k_msleep(RESET_FLUSH_DELAY_MS);

    sys_reboot(SYS_REBOOT_COLD);
}

int main(void)
{
    int err;
    uint32_t boot_reset_cause = 0;
    bool boot_usb_connected;

    debug_printk("System starting...\n");

    /* Capture reset reason and USB state immediately at boot */
    power_log_reset_cause(&boot_reset_cause);
    boot_usb_connected = power_usb_present();
    debug_printk("USB VBUS: %s\n", boot_usb_connected ? "connected" : "not connected");

    /* Output hardware must be up before the state machine enters its
     * boot state */
    err = light_init();
    if (err != 0) {
        fatal_reset("Main LED init", err);
    }

    err = status_led_init();
    if (err != 0) {
        fatal_reset("Status LED init", err);
    }

    /* Non-fatal: battery sampling retries its ADC setup, and the light
     * works without sensors (SMART mode just stays dark) or BLE */
    err = battery_init();
    if (err != 0) {
        debug_printk("Battery ADC init failed: %d\n", err);
    }

    err = sensors_init();
    if (err != 0) {
        debug_printk("Sensor init failed: %d\n", err);
    }

    err = ble_init();
    if (err != 0) {
        debug_printk("BLE init failed: %d\n", err);
    }

    err = button_init();
    if (err != 0) {
        fatal_reset("Button init", err);
    }

#ifdef DEBUG
    /* Scan I2C bus for debugging */
    const struct device *const i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));

    if (device_is_ready(i2c_dev)) {
        debug_printk("Scanning I2C bus during initialization...\n");
        scan_i2c_bus(i2c_dev);
    } else {
        debug_printk("I2C bus not available for scanning\n");
    }
#endif

    /* Boot into IDLE_CHARGING only on a power-on reset with USB present;
     * any other boot (including a wake from System OFF) turns the light on
     * in the first Active Mode */
    enum system_state boot_state = sm_first_active_mode();

    if ((boot_reset_cause & RESET_POR) && boot_usb_connected) {
        debug_printk("USB POR with VBUS connected - booting into IDLE_CHARGING\n");
        boot_state = IDLE_CHARGING;
    }

    sm_init(boot_state, boot_usb_connected);

    power_usb_poll_start();

    /* Enable button interrupts AFTER slow initialization tasks so no
     * spurious events are queued during boot/wake-up */
    err = button_enable();
    if (err != 0) {
        fatal_reset("Button interrupt enable", err);
    }

    debug_printk("Initialization complete. Ready for events...\n");

    /* Main thread becomes the state machine thread */
    sm_run();

    return 0;
}
