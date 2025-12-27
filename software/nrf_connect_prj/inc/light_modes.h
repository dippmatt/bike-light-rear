#ifndef LIGHT_MODES_H
#define LIGHT_MODES_H

/**
 * @brief LED brightness levels
 */
enum led_brightness {
    LED_OFF = 0,      /* 0% duty cycle */
    LED_50_PERCENT,  /* 50% duty cycle */
    LED_100_PERCENT  /* High visibility mode: 50% with periodic 80% flashes */
};

/**
 * @brief Initialize PWM LED hardware
 * 
 * Configures PWM device and sets LED to off state.
 * 
 * @return 0 on success, negative error code on failure
 */
int light_modes_init(void);

/**
 * @brief Set LED brightness to 0% (off)
 * Stops any active blinking mode.
 */
void light_modes_set_off(void);

/**
 * @brief Set LED brightness to 50%
 * Stops any active blinking mode.
 */
void light_modes_set_50_percent(void);

/**
 * @brief Set LED to high visibility blinking mode
 * 
 * LED runs at 50% duty cycle normally, and flashes to 80% for 100ms every second.
 */
void light_modes_set_100_percent(void);

/**
 * @brief Set LED brightness based on enum value
 * 
 * @param brightness Brightness level to set
 */
void light_modes_set_brightness(enum led_brightness brightness);

#endif /* LIGHT_MODES_H */

