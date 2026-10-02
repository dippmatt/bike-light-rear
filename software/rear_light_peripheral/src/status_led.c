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

#include "status_led.h"
#include "debug.h"

#define STATUS_LED_NODE DT_ALIAS(led0)

#define BLINK_INTERVAL_LOW_BATTERY_MS 500
#define BLINK_INTERVAL_CHARGING_MS    300

static const struct gpio_dt_spec status_led = GPIO_DT_SPEC_GET(STATUS_LED_NODE, gpios);

static void blink_timer_expiry(struct k_timer *timer);
static struct k_timer blink_timer;

/* Toggle runs in timer ISR context; GPIO pin access is ISR-safe on nRF52. */
static void blink_timer_expiry(struct k_timer *timer)
{
    ARG_UNUSED(timer);
    gpio_pin_toggle_dt(&status_led);
}

int status_led_init(void)
{
    if (!gpio_is_ready_dt(&status_led)) {
        debug_printk("Error: status LED device not ready\n");
        return -ENODEV;
    }

    int ret = gpio_pin_configure_dt(&status_led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        debug_printk("Error configuring status LED: %d\n", ret);
        return ret;
    }

    k_timer_init(&blink_timer, blink_timer_expiry, NULL);
    return 0;
}

void status_led_set(bool low_battery, bool charging, bool light_on)
{
    /* Stop any blinking first so the timer cannot toggle a freshly set level */
    k_timer_stop(&blink_timer);

    if (low_battery) {
        gpio_pin_set_dt(&status_led, 1);
        k_timer_start(&blink_timer, K_MSEC(BLINK_INTERVAL_LOW_BATTERY_MS),
                      K_MSEC(BLINK_INTERVAL_LOW_BATTERY_MS));
        return;
    }

    if (charging) {
        gpio_pin_set_dt(&status_led, 1);
        k_timer_start(&blink_timer, K_MSEC(BLINK_INTERVAL_CHARGING_MS),
                      K_MSEC(BLINK_INTERVAL_CHARGING_MS));
        return;
    }

    gpio_pin_set_dt(&status_led, light_on ? 1 : 0);
}

void status_led_off(void)
{
    k_timer_stop(&blink_timer);
    gpio_pin_set_dt(&status_led, 0);
}
