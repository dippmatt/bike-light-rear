#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/atomic.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>

#include "ble_central.h"
#include "main_state_machine.h"

#define BLE_SERVICE_UUID       0xA000
#define BLE_LED_STATUS_UUID    0xA001
#define BLE_CONTROL_UUID       0xA002

static struct bt_conn *conn_connected;
static uint16_t svc_start_handle;
static uint16_t svc_end_handle;
static uint16_t led_status_handle;
static uint16_t control_handle;
static bool read_in_progress;
static bool read_ready;
static bool write_in_progress;
static bool discovery_in_progress;
static bool discovery_done;
static uint8_t last_led_status;
static bool state_synced;
static uint8_t stable_value;
static uint8_t stable_count;

#define STABLE_REQUIRED 3U
#define STABLE_WINDOW_MS 3000

static atomic_t manual_pending;
static uint8_t manual_value;

static struct bt_gatt_discover_params discover_params;
static struct bt_gatt_write_params write_params;
static struct bt_gatt_read_params read_params;

static void start_scan(void);
static void ble_tick(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(ble_work, ble_tick);

void ble_central_send_control(uint8_t value)
{
	if (value > LED_SMART_MODE) {
		printk("Ignoring invalid control value: 0x%02x\n", value);
		return;
	}

	manual_value = value;
	atomic_set(&manual_pending, 1);
}

static bool ad_parse_cb(struct bt_data *data, void *user_data)
{
	bool *found = user_data;

	if (data->type != BT_DATA_UUID16_ALL &&
	    data->type != BT_DATA_UUID16_SOME) {
		return true;
	}

	for (size_t i = 0U; i + 1U < data->data_len; i += 2U) {
		uint16_t uuid16 = sys_get_le16(&data->data[i]);

		if (uuid16 == BLE_SERVICE_UUID) {
			*found = true;
			return false;
		}
	}

	return true;
}

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			 struct net_buf_simple *ad)
{
	bool found = false;
	char addr_str[BT_ADDR_LE_STR_LEN];
	int err;

	if (conn_connected) {
		return;
	}

	if (type != BT_GAP_ADV_TYPE_ADV_IND &&
	    type != BT_GAP_ADV_TYPE_ADV_DIRECT_IND) {
		return;
	}

	bt_data_parse(ad, ad_parse_cb, &found);
	if (!found) {
		return;
	}

	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
	printk("Found target: %s (RSSI %d)\n", addr_str, rssi);

	err = bt_le_scan_stop();
	if (err) {
		printk("Failed to stop scan (err %d)\n", err);
		return;
	}

	struct bt_conn *conn = NULL;

	err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN,
				BT_LE_CONN_PARAM_DEFAULT, &conn);
	if (err) {
		printk("Create conn failed (%d)\n", err);
		start_scan();
	} else {
		bt_conn_unref(conn);
	}
}

static void start_scan(void)
{
	int err;

	err = bt_le_scan_start(BT_LE_SCAN_ACTIVE, device_found);
	if (err) {
		printk("Scanning failed to start (err %d)\n", err);
		return;
	}

	printk("Scanning successfully started\n");
}

static void write_cb(struct bt_conn *conn, uint8_t err,
		     struct bt_gatt_write_params *params)
{
	ARG_UNUSED(conn);
	ARG_UNUSED(params);
	write_in_progress = false;

	if (err) {
		printk("Control write cb err %u\n", err);
		return;
	}

	printk("Control write cb ok\n");
}

static void write_control_value(struct bt_conn *conn, uint8_t value)
{
	static uint8_t control_value;
	int err;

	control_value = value;
	write_params.handle = control_handle;
	write_params.offset = 0U;
	write_params.data = &control_value;
	write_params.length = sizeof(control_value);
	write_params.func = write_cb;

	write_in_progress = true;
	err = bt_gatt_write(conn, &write_params);
	if (err) {
		write_in_progress = false;
		printk("Control write failed (err %d)\n", err);
	} else {
		printk("Control value written: 0x%02x\n", control_value);
	}
}

static uint8_t read_cb(struct bt_conn *conn, uint8_t err,
		       struct bt_gatt_read_params *params,
		       const void *data, uint16_t length)
{
	ARG_UNUSED(conn);
	ARG_UNUSED(params);
	read_in_progress = false;

	if (err) {
		printk("Read failed (err %u)\n", err);
		return BT_GATT_ITER_STOP;
	}

	if (!data) {
		return BT_GATT_ITER_STOP;
	}

	if (length >= sizeof(uint8_t)) {
		last_led_status = ((const uint8_t *)data)[0];
		read_ready = true;
	}

	return BT_GATT_ITER_STOP;
}

static uint8_t discover_char_cb(struct bt_conn *conn,
				const struct bt_gatt_attr *attr,
				struct bt_gatt_discover_params *params)
{
	ARG_UNUSED(params);
	ARG_UNUSED(conn);
	if (!attr) {
		if (led_status_handle && control_handle) {
			printk("Characteristics discovered\n");
			discovery_done = true;
		} else {
			printk("Characteristic discovery incomplete\n");
		}
		discovery_in_progress = false;
		return BT_GATT_ITER_STOP;
	}

	const struct bt_gatt_chrc *chrc = attr->user_data;

	if (!bt_uuid_cmp(chrc->uuid, BT_UUID_DECLARE_16(BLE_LED_STATUS_UUID))) {
		led_status_handle = chrc->value_handle;
		printk("LED status handle: 0x%04x\n", led_status_handle);
	} else if (!bt_uuid_cmp(chrc->uuid, BT_UUID_DECLARE_16(BLE_CONTROL_UUID))) {
		control_handle = chrc->value_handle;
		printk("Control handle: 0x%04x\n", control_handle);
	}

	return BT_GATT_ITER_CONTINUE;
}

static uint8_t discover_primary_cb(struct bt_conn *conn,
				   const struct bt_gatt_attr *attr,
				   struct bt_gatt_discover_params *params)
{
	ARG_UNUSED(params);
	int err;

	if (!attr) {
		printk("Service not found\n");
		discovery_in_progress = false;
		return BT_GATT_ITER_STOP;
	}

	const struct bt_gatt_service_val *service = attr->user_data;

	svc_start_handle = attr->handle + 1U;
	svc_end_handle = service->end_handle;

	discover_params.uuid = NULL;
	discover_params.start_handle = svc_start_handle;
	discover_params.end_handle = svc_end_handle;
	discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;
	discover_params.func = discover_char_cb;

	err = bt_gatt_discover(conn, &discover_params);
	if (err) {
		printk("Char discover failed (err %d)\n", err);
		discovery_in_progress = false;
	}

	return BT_GATT_ITER_STOP;
}

static void discover_service(struct bt_conn *conn)
{
	int err;
	static struct bt_uuid_16 svc_uuid = BT_UUID_INIT_16(BLE_SERVICE_UUID);

	discover_params.uuid = &svc_uuid.uuid;
	discover_params.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
	discover_params.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE;
	discover_params.type = BT_GATT_DISCOVER_PRIMARY;
	discover_params.func = discover_primary_cb;

	discovery_in_progress = true;
	discovery_done = false;
	err = bt_gatt_discover(conn, &discover_params);
	if (err) {
		printk("Service discover failed (err %d)\n", err);
		discovery_in_progress = false;
	}
}

static void connected(struct bt_conn *conn, uint8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (err) {
		printk("Failed to connect to %s %u %s\n", addr, err, bt_hci_err_to_str(err));
		start_scan();
		return;
	}

	printk("Connected: %s\n", addr);
	conn_connected = bt_conn_ref(conn);
	discovery_in_progress = false;
	discovery_done = false;
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	if (conn != conn_connected) {
		return;
	}

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	printk("Disconnected: %s, reason 0x%02x %s\n", addr, reason, bt_hci_err_to_str(reason));

	bt_conn_unref(conn_connected);
	conn_connected = NULL;

	svc_start_handle = 0U;
	svc_end_handle = 0U;
	led_status_handle = 0U;
	control_handle = 0U;
	read_in_progress = false;
	read_ready = false;
	write_in_progress = false;
	discovery_in_progress = false;
	discovery_done = false;
	last_led_status = 0U;
	atomic_set(&manual_pending, 0);
	state_synced = false;
	stable_value = 0U;
	stable_count = 0U;

	start_scan();
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

static void ble_tick(struct k_work *work)
{
	struct bt_conn *conn = NULL;
	ARG_UNUSED(work);

	if (conn_connected) {
		conn = bt_conn_ref(conn_connected);
	}

	if (conn) {
		if (!discovery_done && !discovery_in_progress) {
			discover_service(conn);
		} else if (discovery_done && led_status_handle && !read_in_progress) {
			int err;

			read_params.handle_count = 1U;
			read_params.single.handle = led_status_handle;
			read_params.single.offset = 0U;
			read_params.func = read_cb;

			read_in_progress = true;
			err = bt_gatt_read(conn, &read_params);
			if (err) {
				read_in_progress = false;
				printk("Read start failed (err %d)\n", err);
			}
		}

		if (read_ready) {
			read_ready = false;
			printk("LED status read: 0x%02x\n", last_led_status);

			if (last_led_status <= LED_SMART_MODE) {
				if (!state_synced) {
					main_state_machine_set_state(
						(enum system_state)last_led_status);
					state_synced = true;
					stable_value = last_led_status;
					stable_count = 1U;
				} else if (!atomic_get(&manual_pending)) {
					enum system_state current_state = main_state_machine_get_state();

					if (last_led_status == stable_value) {
						if (stable_count < UINT8_MAX) {
							stable_count++;
						}
					} else {
						stable_value = last_led_status;
						stable_count = 1U;
					}

					int64_t last_change = main_state_machine_last_change_ms();
					bool state_stable = (k_uptime_get() - last_change) >= STABLE_WINDOW_MS;

					if (state_stable && stable_count >= STABLE_REQUIRED &&
					    current_state != (enum system_state)last_led_status) {
						main_state_machine_set_state(
							(enum system_state)last_led_status);
					}
				}
			} else {
				printk("Ignoring invalid LED state: 0x%02x\n", last_led_status);
			}

			if (state_synced && atomic_get(&manual_pending)) {
				if (last_led_status == manual_value) {
					atomic_set(&manual_pending, 0);
				}
			}

			if (state_synced && control_handle && !write_in_progress) {
				enum system_state current_state = main_state_machine_get_state();

				if (current_state <= LED_SMART_MODE) {
					write_control_value(conn, (uint8_t)current_state);
				}
			}
		}

		bt_conn_unref(conn);
	}

	k_work_reschedule(&ble_work, K_SECONDS(1));
}

int bluetooth_init(void)
{
	int err;

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return err;
	}

	printk("Bluetooth initialized\n");

	start_scan();
	k_work_schedule(&ble_work, K_NO_WAIT);

	return 0;
}


