/*
 * Copyright (c) 2024
 *
 * SPDX-LICENSE-Identifier: Apache-2.0
 */

#include "main_state_machine.h"
#include "light_modes.h"

/* LED state - cycles through OFF -> 50% -> 100% -> OFF */
static enum led_brightness led_state = LED_OFF;

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
    
    /* Initialize LED state to OFF */
    led_state = LED_OFF;
    light_modes_set_off();
    
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
    /* Cycle MAIN_LED brightness on button press */
    switch (led_state) {
        case LED_OFF:
            /* Turn LED on with 10kHz PWM at 50% duty cycle */
            light_modes_set_50_percent();
            led_state = LED_50_PERCENT;
            break;
        case LED_50_PERCENT:
            /* Turn LED on with high visibility blinking mode (50% with periodic 80% flashes) */
            light_modes_set_100_percent();
            led_state = LED_100_PERCENT;
            break;
        case LED_100_PERCENT:
            /* Turn LED off by setting duty cycle to 0 */
            light_modes_set_off();
            led_state = LED_OFF;
            break;
    }
}

