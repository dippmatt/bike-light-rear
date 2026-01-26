#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>

#include "ble_service.h"

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BLE_SERVICE_UUID)),
};

static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME,
		sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

static void adv_start_work(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(adv_work, adv_start_work);

static void adv_start_work(struct k_work *work)
{
	int err;

	ARG_UNUSED(work);

	err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad),
			      sd, ARRAY_SIZE(sd));
	if (!err) {
		printk("Advertising started\n");
		return;
	}

	if (err == -EALREADY) {
		printk("Advertising already running\n");
		return;
	}

	printk("Advertising start failed (err %d), retrying\n", err);
	k_work_reschedule(&adv_work, K_SECONDS(1));
}

static void connected(struct bt_conn *conn, uint8_t err)
{
	ARG_UNUSED(conn);
	if (err) {
		printk("Connection failed (err 0x%02x)\n", err);
	} else {
		printk("Connected\n");
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	ARG_UNUSED(conn);
	printk("Disconnected (reason 0x%02x)\n", reason);

	k_work_reschedule(&adv_work, K_NO_WAIT);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

int bluetooth_init(void)
{
	int err;

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return err;
	}

	printk("Bluetooth initialized\n");

	ble_service_init();

	k_work_schedule(&adv_work, K_NO_WAIT);

	return 0;
}


