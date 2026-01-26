#ifndef BLE_CENTRAL_H
#define BLE_CENTRAL_H

#include <stdint.h>

int bluetooth_init(void);
void ble_central_send_control(uint8_t value);

#endif /* BLE_CENTRAL_H */


