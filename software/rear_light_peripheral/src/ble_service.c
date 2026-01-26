#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/gatt.h>

#include "ble_service.h"

static uint8_t led_status_value = 0x01;
static uint8_t control_value = 0x00;

static ssize_t read_led_status(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			       void *buf, uint16_t len, uint16_t offset)
{
	const uint8_t *value = attr->user_data;

	ARG_UNUSED(conn);
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(*value));
}

static ssize_t write_control(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			     const void *buf, uint16_t len, uint16_t offset,
			     uint8_t flags)
{
	uint8_t *value = attr->user_data;

	ARG_UNUSED(conn);
	ARG_UNUSED(flags);

	if (offset != 0U || len != sizeof(uint8_t)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	*value = ((const uint8_t *)buf)[0];
	control_value = *value;
	led_status_value = control_value;
	printk("Control value written: 0x%02x\n", control_value);

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


