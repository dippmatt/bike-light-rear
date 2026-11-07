/*
 * USB power detection and reset cause (SPEC USB-1, BOOT-1).
 */

#ifndef POWER_H
#define POWER_H

#include <stdbool.h>
#include <stdint.h>

/** Read and clear the reset reason register; returns the raw flags */
uint32_t power_take_reset_reason(void);

/** A power-on reset (or brownout) leaves no reset-reason flag set */
bool power_reset_is_power_on(uint32_t reason);

bool power_usb_present(void);

/** Start the periodic VBUS poll (ST_USB timer) */
void power_usb_poll_start(void);

/** EVT_USB_POLL: post EVT_USB_CONNECTED / EVT_USB_DISCONNECTED on a change */
void power_usb_poll(void);

#endif /* POWER_H */
