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

#include "main_state_machine.h"
#include "light_modes.h"
#include "utils.h"

/* LED state - cycles through OFF -> 50% -> 100% -> OFF
 * Note: We initialize to LED_50_PERCENT so device is immediately functional after wake-up */
enum led_brightness g_led_state = LED_50_PERCENT;

/**
 * @brief Initialize the main state machine
 * 
 * Sets up the state machine and connects button and LED control.
 * 
 * @return 0 on success, negative error code on failure
 */
int main_state_machine_init(void)
{
    /* Initialize light modes */
    int ret = light_modes_init();
    if (ret != 0) {
        return ret;
    }
    
    /* Initialize LED state to LED_50_PERCENT instead of LED_OFF
     * This ensures that when the device wakes from deep sleep (via button press),
     * it immediately goes to 50% brightness and is ready to use.
     * LED_OFF state always means deep sleep mode. */
    g_led_state = LED_50_PERCENT;
    light_modes_set_50_percent();
    
    return 0;
}

/**
 * @brief Handle button press event
 * 
 * Called by button module when a button press is detected.
 * This function cycles through LED brightness states.
 */
void main_state_machine_on_button_press(void)
{
    debug_printk("State machine: Button press handler called, current state: %d\n", g_led_state);
    /* Cycle through LED brightness states: OFF -> 50% -> 50_80_FLASH -> SMART_MODE -> OFF */
    switch (g_led_state) {
        case LED_OFF:
            /* Turn LED on with 10kHz PWM at 50% duty cycle */
            light_modes_set_50_percent();
            g_led_state = LED_50_PERCENT;
            break;
        case LED_50_PERCENT:
            /* Turn LED on with high visibility blinking mode (50% with periodic 80% flashes) */
            light_modes_set_100_percent();
            g_led_state = LED_50_80_FLASH;
            break;
        case LED_50_80_FLASH:
            /* Enter SMART_MODE */
            light_modes_set_smart_mode();
            g_led_state = LED_SMART_MODE;
            break;
        case LED_SMART_MODE:
            /* Turn LED off by setting duty cycle to 0 */
            light_modes_set_off();
            g_led_state = LED_OFF;
            break;
    }
    /* Note: Status LED blinking is controlled by timer callback which checks the state */
}

/**
 * @brief Automatically turn off LED
 * 
 * Called by stationary monitor when device has been stationary for too long.
 * Only acts if current state is SMART_MODE.
 * 
 * @param enter_sleep If true, system will enter sleep mode after turning off
 */
void main_state_machine_auto_off(bool enter_sleep)
{
    ARG_UNUSED(enter_sleep);
    
    /* Only auto-off from SMART_MODE */
    if (g_led_state == LED_SMART_MODE) {
        debug_printk("State machine: Auto-off triggered, current state: %d\n", g_led_state);
        light_modes_set_off();
        g_led_state = LED_OFF;
        
        /* Note: Sleep functionality would be triggered by caller if enter_sleep is true */
        /* This is left to main.c to handle since it owns the sleep functionality */
    }
}

/**
 * @brief Get current LED state
 * 
 * @return Current LED brightness state
 */
enum led_brightness main_state_machine_get_state(void)
{
    return g_led_state;
}

