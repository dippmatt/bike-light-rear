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
#include "sensor_data_collector.h"
#include "utils.h"

/* LED state - cycles through OFF -> 50% -> 50_80_FLASH -> SMART_MODE -> OFF */
enum system_state g_system_state = LED_50_PERCENT;

/* Forward declarations for state init/terminate functions */
static void state_led_50_percent_init(void);
static void state_led_50_percent_terminate(void);
static void state_led_50_80_flash_init(void);
static void state_led_50_80_flash_terminate(void);
static void state_led_smart_mode_init(void);
static void state_led_smart_mode_terminate(void);
static void state_led_off_init(void);
static void state_led_off_terminate(void);

/* State function pointer types */
typedef void (*state_init_func_t)(void);
typedef void (*state_terminate_func_t)(void);

/* State function table */
static const struct {
    state_init_func_t init;
    state_terminate_func_t terminate;
} state_functions[] = {
    [LED_OFF] = {state_led_off_init, state_led_off_terminate},
    [LED_50_PERCENT] = {state_led_50_percent_init, state_led_50_percent_terminate},
    [LED_50_80_FLASH] = {state_led_50_80_flash_init, state_led_50_80_flash_terminate},
    [LED_SMART_MODE] = {state_led_smart_mode_init, state_led_smart_mode_terminate},
};

/**
 * @brief General state transition function
 * 
 * Handles the transition from one state to another by calling
 * the terminate function of the current state and the init function
 * of the next state.
 * 
 * @param from_state Current state (will be terminated)
 * @param to_state Next state (will be initialized)
 */
static void state_transition(enum system_state from_state, enum system_state to_state)
{
    debug_printk("State transition: %d -> %d\n", from_state, to_state);
    
    /* Terminate current state */
    if (from_state < sizeof(state_functions) / sizeof(state_functions[0]) &&
        state_functions[from_state].terminate != NULL) {
        state_functions[from_state].terminate();
    }
    
    /* Update global state */
    g_system_state = to_state;
    
    /* Initialize next state */
    if (to_state < sizeof(state_functions) / sizeof(state_functions[0]) &&
        state_functions[to_state].init != NULL) {
        state_functions[to_state].init();
    }
}

/**
 * @brief Initialize LED_50_PERCENT state
 * 
 * Sets up PWM at 50% duty cycle.
 */
static void state_led_50_percent_init(void)
{
    debug_printk("State init: LED_50_PERCENT\n");
    light_modes_start_status_led();
    light_modes_set_50_percent();
}

/**
 * @brief Terminate LED_50_PERCENT state
 * 
 * Sets LED pin to low (turns off).
 */
static void state_led_50_percent_terminate(void)
{
    debug_printk("State terminate: LED_50_PERCENT\n");
    /* LED will be controlled by next state, no cleanup needed */
}

/**
 * @brief Initialize LED_50_80_FLASH state
 * 
 * Sets up two timers for blinking mode and initializes PWM.
 */
static void state_led_50_80_flash_init(void)
{
    debug_printk("State init: LED_50_80_FLASH\n");
    light_modes_set_100_percent();
}

/**
 * @brief Terminate LED_50_80_FLASH state
 * 
 * Stops both timers and turns off LED.
 */
static void state_led_50_80_flash_terminate(void)
{
    debug_printk("State terminate: LED_50_80_FLASH\n");
    light_modes_stop_blinking();
}

/**
 * @brief Initialize LED_SMART_MODE state
 * 
 * Sets up smart mode and starts sensor sampling.
 */
static void state_led_smart_mode_init(void)
{
    debug_printk("State init: LED_SMART_MODE\n");
    light_modes_set_smart_mode();
    sensor_data_collector_start();
}

/**
 * @brief Terminate LED_SMART_MODE state
 * 
 * Stops sensor sampling and turns off LED.
 */
static void state_led_smart_mode_terminate(void)
{
    debug_printk("State terminate: LED_SMART_MODE\n");
    sensor_data_collector_stop();
}

/**
 * @brief Initialize LED_OFF state
 * 
 * Ensures LED is off.
 */
static void state_led_off_init(void)
{
    debug_printk("State init: LED_OFF\n");
    debug_printk("Stopping status LED\n");
    light_modes_stop_status_led();
    debug_printk("Turning off main LED\n");
    light_modes_off_main_led();
}

/**
 * @brief Terminate LED_OFF state
 * 
 * No cleanup needed - LED is already off.
 */
static void state_led_off_terminate(void)
{
    debug_printk("State terminate: LED_OFF\n");
    /* LED is already off, no cleanup needed */
}

/**
 * @brief Initialize the main state machine
 * 
 * Sets up the state machine and initializes to LED_50_PERCENT state.
 * 
 * @return 0 on success, negative error code on failure
 */
int main_state_machine_init(void)
{
    int ret;
    
    /* Initialize flash timers for blinking mode */
    ret = flash_timers_init();
    if (ret != 0) {
        return ret;
    }
    
    /* Initialize status LED */
    ret = status_led_init();
    if (ret != 0) {
        return ret;
    }
    
    /* Initialize to LED_50_PERCENT state */
    g_system_state = LED_50_PERCENT;
    state_led_50_percent_init();
    
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
    enum system_state current_state = g_system_state;
    enum system_state next_state;
    
    debug_printk("State machine: Button press handler called, current state: %d\n", current_state);
    
    /* Determine next state based on current state */
    switch (current_state) {
        case LED_OFF:
            next_state = LED_50_PERCENT;
            break;
        case LED_50_PERCENT:
            next_state = LED_50_80_FLASH;
            break;
        case LED_50_80_FLASH:
            next_state = LED_SMART_MODE;
            break;
        case LED_SMART_MODE:
            next_state = LED_OFF;
            break;
        default:
            /* Invalid state, reset to LED_50_PERCENT */
            next_state = LED_50_PERCENT;
            break;
    }
    
    /* Perform state transition */
    state_transition(current_state, next_state);
}

/**
 * @brief Automatically turn off LED
 * 
 * Called by stationary monitor when device has been stationary for too long.
 * Only acts if current state is SMART_MODE.
 * 
 */
void main_state_machine_auto_off()
{    
    debug_printk("State machine: Auto-off triggered, current state: %d\n", g_system_state);
    state_transition(LED_SMART_MODE, LED_OFF);
}

/**
 * @brief Get current LED state
 * 
 * @return Current LED brightness state
 */
enum system_state main_state_machine_get_state(void)
{
    return g_system_state;
}

/**
 * @brief Set LED state directly (for external control, e.g., BLE)
 * 
 * Allows external modules to directly set the LED state.
 * This bypasses the normal button press cycle.
 * 
 * @param new_state The state to transition to
 * @return 0 on success, negative error code on failure
 */
int main_state_machine_set_state(enum system_state new_state)
{
    /* Validate state */
    if (new_state > LED_SMART_MODE) {
        return -EINVAL;
    }
    
    enum system_state current_state = g_system_state;
    
    /* Only transition if state is different */
    if (current_state != new_state) {
        state_transition(current_state, new_state);
    }
    
    return 0;
}
