/*
 * Copyright (c) 2016 Intel Corporation
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

 #include <hal/nrf_gpio.h>
 #include <zephyr/kernel.h>
 
 #define PIN_11 11  // Active low (LOW = active, HIGH = inactive)
 #define PIN_09 9   // Active high (HIGH = active, LOW = inactive)
 #define PIN_BUTTON 32  // P1.00 = 32 (button input with pullup)
 
 #define ACTIVE_TIME_MS   100 / 20 // 100ms active time
 #define INACTIVE_TIME_MS 1000 / 20 // 1000ms (1s) inactive time
 #define BUTTON_POLL_MS   10       // Poll button every 10ms
 
 /* Timer for controlling pin 11 blinking */
 static struct k_timer blink_timer;
 static bool pin11_is_active = false;
 
 /* Timer callback function - runs in interrupt context */
 static void blink_timer_handler(struct k_timer *timer)
 {
     if (pin11_is_active) {
         /* Pin 11 is currently active, make it inactive */
         nrf_gpio_pin_write(PIN_11, 1);  // Inactive (HIGH)
         pin11_is_active = false;
         
         /* Schedule next activation after inactive period */
         k_timer_start(&blink_timer, K_MSEC(INACTIVE_TIME_MS), K_NO_WAIT);
     } else {
         /* Pin 11 is currently inactive, make it active */
         nrf_gpio_pin_write(PIN_11, 0);  // Active (LOW)
         pin11_is_active = true;
         
         /* Schedule next deactivation after active period */
         k_timer_start(&blink_timer, K_MSEC(ACTIVE_TIME_MS), K_NO_WAIT);
     }
 }
 
 int main(void)
 {
     /* Configure GPIO pins */
     nrf_gpio_cfg_output(PIN_11);  // Configure P0.11 as output
     nrf_gpio_cfg_output(PIN_09);  // Configure P0.09 as output
     nrf_gpio_cfg_input(PIN_BUTTON, NRF_GPIO_PIN_PULLUP);  // Configure P1.00 as input with pullup
     
     /* Initialize pin states */
     nrf_gpio_pin_write(PIN_11, 1);  // P0.11 inactive (HIGH, active low)
     nrf_gpio_pin_write(PIN_09, 0);  // P0.09 starts LOW (inactive)
     
     /* Initialize and start the timer */
     k_timer_init(&blink_timer, blink_timer_handler, NULL);
     
     /* Start the first cycle - begin with inactive period */
     k_timer_start(&blink_timer, K_MSEC(INACTIVE_TIME_MS), K_NO_WAIT);
     
     /* Main loop polls button and controls pin 09 */
     while (1) {
         /* Read button state (active low - pressed = 0, released = 1) */
         uint32_t button_state = nrf_gpio_pin_read(PIN_BUTTON);
         
         if (button_state == 0) {
             /* Button pressed - set pin 09 HIGH (active) */
             nrf_gpio_pin_write(PIN_09, 1);
         } else {
             /* Button released - set pin 09 LOW (inactive) */
             nrf_gpio_pin_write(PIN_09, 0);
         }
         
         /* Poll button at regular intervals */
         k_msleep(BUTTON_POLL_MS);
     }
 
     return 0;
 }
 