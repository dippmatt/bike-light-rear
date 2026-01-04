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

/* Global sensor buffer and environmental state */
sensor_buffer_t g_sensor_buffer;
environmental_state_t g_env_state;

/* GPIO Devicetree Specifications */
#define STATUS_LED_NODE     DT_ALIAS(led0)

/* Timer interval for STATUS_LED blinking */
#define TIMER_INTERVAL_MS 500

/* GPIO device specification for STATUS_LED */
static const struct gpio_dt_spec status_led = GPIO_DT_SPEC_GET(STATUS_LED_NODE, gpios);

/* Timer for STATUS_LED blinking */
static struct k_timer led_timer;

/**
 * @brief STATUS_LED timer expiry callback
 * 
 * Toggles the LED on every timer tick, but only if not in LED_OFF mode.
 * When in LED_OFF mode, the LED stays off.
 */
static void led_timer_expiry()
{
    /* Only blink if not in LED_OFF mode */
    if (main_state_machine_get_state() != LED_OFF) {
        gpio_pin_toggle_dt(&status_led);
    } else {
        /* Ensure LED is off when in LED_OFF mode */
        gpio_pin_set_dt(&status_led, 0);
    }
}

static char *helper_button_evt_str(enum button_evt evt)
{
	switch (evt) {
	case BUTTON_EVT_PRESSED:
		return "Pressed";
	case BUTTON_EVT_RELEASED:
		return "Released";
	default:
		return "Unknown";
	}
}

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
	
	/* Stop the LED timer to save power */
	k_timer_stop(&led_timer);
	
	/* Ensure status LED is off before sleep */
	gpio_pin_set_dt(&status_led, 0);
	
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

static void button_event_handler(enum button_evt evt)
{
	debug_printk("Button event: %s\n", helper_button_evt_str(evt));

    /* Advance state machine on button press */
	if (evt == BUTTON_EVT_PRESSED) {
		enum led_brightness prev_state = main_state_machine_get_state();
		main_state_machine_on_button_press();
		enum led_brightness new_state = main_state_machine_get_state();
		
		/* If we transitioned to LED_OFF, enter sleep mode */
		if (new_state == LED_OFF && prev_state != LED_OFF) {
			debug_printk("Transitioning to LED_OFF - entering sleep mode\n");
			/* Give a short delay for user feedback (status LED off) */
			k_msleep(100);
			power_management_enter_sleep();
		}
	}
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
    
    /* Check if STATUS_LED device is ready */
    if (!gpio_is_ready_dt(&status_led)) {
        return -1;
    }
    /* Configure STATUS_LED as output and initialize to off */
    ret = gpio_pin_configure_dt(&status_led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        return -1;
    }

    /* Initialize main state machine (LED control) */
    ret = main_state_machine_init();
    if (ret != 0) {
        /* State machine initialization failed */
        return -1;
    }
    
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
    
    /* Initialize and start LED blink timer */
    /* Timer fires every 500ms, toggling the LED for 1Hz blink (on 500ms, off 500ms) */
    /* Note: Timer callback checks state and won't blink when in LED_OFF mode */
    k_timer_init(&led_timer, led_timer_expiry, NULL);
    k_timer_start(&led_timer, K_MSEC(TIMER_INTERVAL_MS), K_MSEC(TIMER_INTERVAL_MS));
    
    /* Update status LED based on current state */
    if (main_state_machine_get_state() == LED_OFF) {
        gpio_pin_set_dt(&status_led, 0);
    }
    
    /* Check if we're in LED_OFF state - if so, immediately enter deep sleep
     * This ensures the device never stays awake in LED_OFF mode.
     * LED_OFF always means deep sleep. */
    if (main_state_machine_get_state() == LED_OFF) {
        debug_printk("Starting in LED_OFF mode - entering deep sleep immediately\n");
        k_msleep(100);  /* Small delay to allow initialization to complete */
        power_management_enter_sleep();
        /* Never returns - system will wake on button press */
    }
    
    /* Main loop (only reached if NOT in LED_OFF state) */
    while (1) {
        /* Sleep to save power - timers and interrupts will wake the CPU */
        k_msleep(1000);
    }

    return 0;
}
