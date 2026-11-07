/*
 * Rear light board pin map (BL653 / nRF52833), from the hardware design
 * (the same assignment as nrf52833dk_nrf52833.overlay of the Zephyr firmware).
 * Tunable constants are in config.h.
 */

#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

#define PIN_MAIN_LED   9U  /* P0.09, PWM, active high (NFC pin, needs UICR NFCPINS = GPIO) */
#define PIN_STATUS_LED 11U /* P0.11, active low */
#define PIN_BUTTON     0U  /* P1.00, active low with pull-up */
#define PIN_I2C_SDA    26U /* P0.26 */
#define PIN_I2C_SCL    27U /* P0.27 */

#define I2C_ADDR_LIS3DH  0x19U /* SA0 tied to VDD */
#define I2C_ADDR_OPT3001 0x44U

/* Battery divider: 1M/100k, SAADC internal 0.6 V reference, 12 bit */
#define BATTERY_DIVIDER_RATIO 11U

#endif /* BOARD_H */
