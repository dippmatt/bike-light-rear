/*
 * Blocking I2C master on TWIM0 (SDA P0.26, SCL P0.27, 400 kHz). A transfer
 * takes well under a millisecond, so polling is acceptable in the main loop.
 * Buffers must be in RAM (EasyDMA).
 */

#ifndef TWIM_H
#define TWIM_H

#include <stdint.h>

/** Power up the peripheral and configure the pins */
void twim_enable(void);

/** Power down the peripheral and release the pins (Deep Sleep, sensors stopped) */
void twim_disable(void);

/** @return 0 on success, negative on a NACK, an error or a timeout */
int twim_write(uint8_t address, const uint8_t *data, uint8_t length);

/** Write @p tx_length bytes (register pointer), repeated start, read @p rx_length bytes */
int twim_write_read(uint8_t address, const uint8_t *tx, uint8_t tx_length, uint8_t *rx,
                    uint8_t rx_length);

#endif /* TWIM_H */
