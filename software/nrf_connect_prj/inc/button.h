#ifndef BUTTON_H
#define BUTTON_H

#include <zephyr/kernel.h>
#include <stdbool.h>

/**
 * @brief Button press callback function type
 * 
 * This callback is called when a button press is detected (after debounce).
 */
typedef void (*button_press_callback_t)(void);

/**
 * @brief Initialize button hardware and detection
 * 
 * Configures GPIO pin, sets up interrupts, and initializes debounce timer.
 * 
 * @param callback Function to call when button press is detected
 * @return 0 on success, negative error code on failure
 */
int button_init(button_press_callback_t callback);

#endif /* BUTTON_H */

