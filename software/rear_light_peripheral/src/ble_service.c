#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/gatt.h>

#include "ble_service.h"
#include "main_state_machine.h"
#include "utils.h"

#define STABLE_WINDOW_MS 3000

static uint8_t led_status_value = 0x01;
static uint8_t control_value = 0x00;
static uint8_t last_control_value = 0x00;

static ssize_t read_led_status(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			       void *buf, uint16_t len, uint16_t offset)
{
	uint8_t current_state = (uint8_t)main_state_machine_get_state();
	led_status_value = current_state;

	ARG_UNUSED(conn);
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &led_status_value,
				 sizeof(led_status_value));
}

static ssize_t write_control(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			     const void *buf, uint16_t len, uint16_t offset,
			     uint8_t flags)
{
	uint8_t *value = attr->user_data;
	enum system_state current_state;
	int64_t last_change;
	bool state_stable;

	ARG_UNUSED(conn);
	ARG_UNUSED(flags);

	if (offset != 0U || len != sizeof(uint8_t)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	*value = ((const uint8_t *)buf)[0];
	control_value = *value;

	if (control_value <= IDLE_CHARGING) {
		current_state = main_state_machine_get_state();
		if (control_value == current_state) {
			last_control_value = control_value;
			return len;
		}

		last_change = main_state_machine_last_change_ms();
		state_stable = (k_uptime_get() - last_change) >= STABLE_WINDOW_MS;
		if (!state_stable) {
			if (control_value != last_control_value) {
				printk("Control ignored (local change < %d ms): 0x%02x\n",
				       STABLE_WINDOW_MS, control_value);
				last_control_value = control_value;
			}
			return len;
		}

		if (control_value != last_control_value) {
			printk("Control value changed: 0x%02x\n", control_value);
			last_control_value = control_value;
		}

		if (control_value != current_state) {
			main_state_machine_set_state((enum system_state)control_value);
		}

		led_status_value = (uint8_t)main_state_machine_get_state();
	} else {
		debug_printk("Invalid control value: 0x%02x\n", control_value);
	}

	return len;
}

BT_GATT_SERVICE_DEFINE(ble_svc,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_DECLARE_16(BLE_SERVICE_UUID)),
	BT_GATT_CHARACTERISTIC(BT_UUID_DECLARE_16(BLE_LED_STATUS_UUID),
			       BT_GATT_CHRC_READ,
			       BT_GATT_PERM_READ,
			       read_led_status, NULL, &led_status_value),
	BT_GATT_CHARACTERISTIC(BT_UUID_DECLARE_16(BLE_CONTROL_UUID),
			       BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_WRITE,
			       NULL, write_control, &control_value),
);

int ble_service_init(void)
{
	return 0;
}

int ble_service_set_led_status(uint8_t value)
{
	led_status_value = value;
	return 0;
}

uint8_t ble_service_get_control_value(void)
{
	return control_value;
}


