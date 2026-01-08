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
#include <zephyr/devicetree.h>
#include <zephyr/pm/device.h>
#include <zephyr/sys/poweroff.h>

#include "power_management.h"
#include "button.h"
#include "light_modes.h"
#include "utils.h"

/**
 * @brief Enter system off (deep sleep) mode
 * 
 * Prepares the system for low-power system off mode and enters it.
 * The system will wake on button press (configured as wake source).
 * Note: System off is like a reset - upon wake, execution starts from main().
 */
void power_management_enter_sleep(void)
{
	int ret;
	
	debug_printk("Entering system off mode (deep sleep)...\n");
	
	/* Stop the status LED timer to save power */
	light_modes_stop_status_led();
	
	/* Configure button as wakeup source (level-active interrupt) */
	ret = button_configure_wakeup();
	if (ret < 0) {
		debug_printk("Error: Failed to configure button wakeup (%d)\n", ret);
		return;
	}
	
	/* Suspend console device to save power */
	const struct device *const cons = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	if (device_is_ready(cons)) {
		int rc = pm_device_action_run(cons, PM_DEVICE_ACTION_SUSPEND);
		if (rc < 0) {
			debug_printk("Warning: Could not suspend console (%d)\n", rc);
		}
	}
	
	/* Small delay to ensure debug message is sent */
	k_msleep(50);
	
	/* Enter system off - button interrupt will wake the system */
	sys_poweroff();
	
	/* Code never reaches here - system resets on wake */
}
