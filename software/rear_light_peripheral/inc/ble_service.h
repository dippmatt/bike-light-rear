#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include <stdint.h>

#define BLE_SERVICE_UUID       0xA000
#define BLE_LED_STATUS_UUID    0xA001
#define BLE_CONTROL_UUID       0xA002

int ble_service_init(void);
int ble_service_set_led_status(uint8_t value);
uint8_t ble_service_get_control_value(void);

#endif /* BLE_SERVICE_H */


