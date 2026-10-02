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

#include "light.h"
#include "debug.h"

#define MAIN_LED_PWM_NODE DT_ALIAS(main_led0)

static const struct pwm_dt_spec main_led = PWM_DT_SPEC_GET(MAIN_LED_PWM_NODE);

int light_init(void)
{
    if (!pwm_is_ready_dt(&main_led)) {
        debug_printk("Error: main LED PWM device not ready\n");
        return -ENODEV;
    }

    /* Set period and start with the LED off */
    return pwm_set_dt(&main_led, PWM_USEC(LIGHT_PWM_PERIOD_USEC), 0);
}

void light_set(uint32_t pulse_usec)
{
    int ret = pwm_set_pulse_dt(&main_led, PWM_USEC(pulse_usec));

    if (ret < 0) {
        debug_printk("Error setting main LED pulse to %u us: %d\n", pulse_usec, ret);
        return;
    }
    debug_printk("Main LED pulse set to %u us\n", pulse_usec);
}
