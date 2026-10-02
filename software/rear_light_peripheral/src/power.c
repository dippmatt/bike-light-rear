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
#include <zephyr/pm/device.h>
#include <zephyr/sys/poweroff.h>
#include <zephyr/drivers/hwinfo.h>
#include <hal/nrf_power.h>

#include "power.h"
#include "app_events.h"
#include "debug.h"

#define USB_POLL_INTERVAL_MS 1000

static struct k_timer usb_poll_timer;
/* Only touched from the poll timer ISR (after init) */
static bool usb_present_prev;

bool power_usb_present(void)
{
    uint32_t usbregstatus = NRF_POWER->USBREGSTATUS;

    return (usbregstatus & POWER_USBREGSTATUS_VBUSDETECT_Msk) != 0;
}

/* Timer ISR: read the VBUS register (plain register read, ISR-safe) and
 * post edge events only when the state changes. */
static void usb_poll_timer_expiry(struct k_timer *timer)
{
    ARG_UNUSED(timer);

    bool present = power_usb_present();

    if (present != usb_present_prev) {
        usb_present_prev = present;
        app_event_post(present ? EVT_USB_CONNECTED : EVT_USB_DISCONNECTED, 0);
    }
}

void power_usb_poll_start(void)
{
    usb_present_prev = power_usb_present();

    k_timer_init(&usb_poll_timer, usb_poll_timer_expiry, NULL);
    k_timer_start(&usb_poll_timer, K_MSEC(USB_POLL_INTERVAL_MS),
                  K_MSEC(USB_POLL_INTERVAL_MS));
}

void power_log_reset_cause(uint32_t *cause_out)
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

FUNC_NORETURN void power_off_finalize(void)
{
    debug_printk("Entering system off mode (deep sleep)...\n");

    /* Suspend console device to save power */
    const struct device *const cons = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

    if (device_is_ready(cons)) {
        int rc = pm_device_action_run(cons, PM_DEVICE_ACTION_SUSPEND);
        if (rc < 0) {
            debug_printk("Warning: Could not suspend console (%d)\n", rc);
        }
    }

    /* Small delay to ensure the last debug message is sent */
    k_msleep(50);

    /* Wake-up (button level interrupt) resets the system into main() */
    sys_poweroff();
}
