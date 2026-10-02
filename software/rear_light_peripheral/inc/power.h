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

#ifndef POWER_H
#define POWER_H

#include <stdbool.h>
#include <stdint.h>
#include <zephyr/toolchain.h>

/**
 * Power: USB VBUS detection, reset-cause logging, System OFF entry.
 */

/**
 * @brief Read the current USB VBUS connection state
 *
 * @return true if VBUS is present
 */
bool power_usb_present(void);

/**
 * @brief Start periodic (1 s) USB VBUS monitoring
 *
 * Posts EVT_USB_CONNECTED / EVT_USB_DISCONNECTED edge events to the
 * central event queue when the VBUS state changes.
 */
void power_usb_poll_start(void);

/**
 * @brief Log and clear the hardware reset cause
 *
 * Call once, immediately at boot.
 *
 * @param cause_out If non-NULL, the raw reset cause flags are stored here
 */
void power_log_reset_cause(uint32_t *cause_out);

/**
 * @brief Suspend the console and enter System OFF (deep sleep)
 *
 * The button must already be configured as wake source
 * (button_prepare_wake()). Wake-up resets the system into main().
 */
FUNC_NORETURN void power_off_finalize(void);

#endif /* POWER_H */
