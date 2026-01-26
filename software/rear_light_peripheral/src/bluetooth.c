/*
 * Copyright (c) 2025 Matthias Dippold
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * NON-COMMERCIAL USE CLAUSE:
 * Permission is granted to use, copy, modify, and distribute this software
 * and its derivatives for NON-COMMERCIAL purposes only. Commercial use of
 * this software, in whole or in part, is strictly prohibited without
 * explicit written permission from the copyright holder.
 */

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/sys/util.h>
#include <string.h>

#include "bluetooth.h"
#include "main_state_machine.h"
#include "light_modes.h"
#include "utils.h"

#define DEVICE_NAME             CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN         (sizeof(DEVICE_NAME) - 1)

#ifdef BLUETOOTH
#ifndef BLE_MASTER
/* ========== BLE PERIPHERAL / SLAVE MODE IMPLEMENTATION ========== */

static enum system_state led_ble_state = LED_50_PERCENT;

#define LED_SERVICE_UUID_VAL \
  BT_UUID_128_ENCODE(0xf7547938, 0x68ba, 0x11ec, 0x90d6, 0x0242ac120003)

// Service and Characteristics UUIDs
static struct bt_uuid_128 led_state_char_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0x9c85a726, 0xb7f1, 0x11ec, 0xb909, 0x0242ac120002));

static struct bt_uuid_128 led_svc_uuid =
	BT_UUID_INIT_128(LED_SERVICE_UUID_VAL);

static ssize_t read_led_state(struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    void *buf, uint16_t len, uint16_t offset) {
    uint8_t current_state = (uint8_t)main_state_machine_get_state();
    debug_printk("Value 0x%x read.\n", current_state);
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &current_state,
                             sizeof(current_state));
}

static ssize_t write_led_state(struct bt_conn *conn,
     const struct bt_gatt_attr *attr, const void *buf,
     uint16_t len, uint16_t offset, uint8_t flags) {
     ARG_UNUSED(conn);
     ARG_UNUSED(offset);
     ARG_UNUSED(flags);

     uint8_t *val = attr->user_data;
     *val = *((uint8_t *)buf);

     debug_printk("Value 0x%x written.\n", *val);


     led_ble_state = main_state_machine_get_state();

     switch(led_ble_state) {
      case LED_OFF:
          debug_printk("LED_OFF\n");
          break;
      case LED_50_PERCENT:
          debug_printk("Current LED state LED_50_PERCENT advacing to LED_50_80_FLASH\n");
          main_state_machine_set_state(LED_50_80_FLASH);
          break;
      case LED_50_80_FLASH:
          debug_printk("Current LED state LED_50_80_FLASH advacing to LED_SMART_MODE\n");
          main_state_machine_set_state(LED_SMART_MODE);
          break;
      case LED_SMART_MODE:
          debug_printk("Current LED state LED_SMART_MODE advacing to LED_OFF\n");
          main_state_machine_set_state(LED_OFF);
          break;
      default:
          debug_printk("Unknown LED state\n");
          break;
  }
    
     //gpio_pin_set_dt(&led, led_state);

     return len;
}

BT_GATT_SERVICE_DEFINE(
    led_svc, BT_GATT_PRIMARY_SERVICE(&led_svc_uuid),
    BT_GATT_CHARACTERISTIC(&led_state_char_uuid.uuid,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                           read_led_state, write_led_state, &led_ble_state), );

// Advertisement Data

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, LED_SERVICE_UUID_VAL),
};
static const struct bt_le_adv_param adv_param = {
    .id = BT_ID_DEFAULT,
    .sid = 0,
    .secondary_max_skip = 0,
    .options = BT_LE_ADV_OPT_CONN,
    .interval_min = BT_GAP_ADV_FAST_INT_MIN_2,
    .interval_max = BT_GAP_ADV_FAST_INT_MAX_2,
};

// Scan Response Data
static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, LED_SERVICE_UUID_VAL),
};

// initialize BLE
int bluetooth_init(void)
{
    int err;
    
    err = bt_enable(NULL);
    if (err) {
        debug_printk("Bluetooth init failed (err %d)\n", err);
        return err;
    }
    debug_printk("Bluetooth initialized\n");

    // start advertising
    err = bt_le_adv_start(&adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    //err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
    //err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));

    if (err) {
        debug_printk("Advertising failed to start (err %d)\n", err);
        return err;
    }
    debug_printk("Advertising successfully started\n");
    
    return 0;
}

static void connected(struct bt_conn *conn, uint8_t err) {
    ARG_UNUSED(conn);
    if (err) {
        debug_printk("Connection failed (err 0x%02x)\n", err);
    } else {
        debug_printk("Connected\n");
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason) {
    ARG_UNUSED(conn);
    debug_printk("Disconnected (reason 0x%02x)\n", reason);

    int err = bt_le_adv_start(&adv_param, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        debug_printk("Advertising restart failed (err %d)\n", err);
    } else {
        debug_printk("Advertising restarted\n");
    }
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};

#else
/* ========== BLE CENTRAL / MASTER MODE IMPLEMENTATION ========== */

// Worker
K_SEM_DEFINE(sem, 1, 1);
static struct k_work work;

// Events
K_EVENT_DEFINE(event);
enum {
  EV_LED_FOUND = BIT(1),
  EV_CONNECTED = BIT(2),
  EV_SERVICE_FOUND = BIT(3),
  EV_HANDLE_FOUND = BIT(4),
  EV_STATE_READ = BIT(5),
  EV_STATE_WRITTEN = BIT(6),
  EV_DISCONNECTED = BIT(7),
};
#define EVENT_TIMEOUT_MS 3000
#define DISCOVERY_TIMEOUT_MS 7000

// Service and Characteristics UUIDs
#define LED_SERVICE_UUID_VAL \
  BT_UUID_128_ENCODE(0xf7547938, 0x68ba, 0x11ec, 0x90d6, 0x0242ac120003)

static struct bt_uuid_128 led_state_char_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0x9c85a726, 0xb7f1, 0x11ec, 0xb909, 0x0242ac120002));

static struct bt_uuid_128 led_svc_uuid = BT_UUID_INIT_128(LED_SERVICE_UUID_VAL);

static uint8_t led_state;
static uint16_t led_state_handle;
static uint16_t led_svc_start_handle;
static uint16_t led_svc_end_handle;
static bt_addr_le_t led_address;
static struct bt_conn *led_conn;
static struct bt_gatt_discover_params discover_params;
static struct bt_gatt_discover_params svc_discover_params;
static int last_read_err;
static int last_write_err;
static bool led_service_found;

static bool wait_for_event(uint32_t ev, uint32_t timeout_ms) {
    uint32_t deadline = k_uptime_get_32() + timeout_ms;

    while (true) {
        uint32_t now = k_uptime_get_32();
        if ((int32_t)(deadline - now) <= 0) {
            break;
        }

        uint32_t remaining = deadline - now;
        uint32_t slice_ms = MIN(remaining, 200U);
        uint32_t got = k_event_wait(&event, ev, false, K_MSEC(slice_ms));

        if (got & ev) {
            return true;
        }
    }

    if (k_event_test(&event, ev)) {
        return true;
    }

    uint32_t grace_deadline = k_uptime_get_32() + 200U;
    while (true) {
        if (k_event_test(&event, ev)) {
            return true;
        }
        if ((int32_t)(grace_deadline - k_uptime_get_32()) <= 0) {
            break;
        }
        k_msleep(20);
    }

    return false;
}

static void stop_scan() {
    int err = bt_le_scan_stop();
    if (err) {
        debug_printk("Stop LE scan failed (err %d)\n", err);
    } else {
        debug_printk("Scanning successfully stopped.\n");
    }
}

static void connect() {
    int err;

    err = bt_conn_le_create(&led_address, BT_CONN_LE_CREATE_CONN,
                            BT_LE_CONN_PARAM_DEFAULT, &led_conn);
    if (err) {
      debug_printk("Create conn failed (err %d)\n", err);
  }
}

static uint8_t led_primary_service_discover_cb(struct bt_conn *conn,
                                               const struct bt_gatt_attr *attr,
                                               struct bt_gatt_discover_params *prms) {
    ARG_UNUSED(conn);
    ARG_UNUSED(prms);

    if (!attr) {
        if (!led_service_found) {
            debug_printk("Service discovery completed without match\n");
        }
        return BT_GATT_ITER_STOP;
    }

    const struct bt_gatt_service_val *service =
        (const struct bt_gatt_service_val *)attr->user_data;
    if (!service || !service->uuid) {
        return BT_GATT_ITER_CONTINUE;
    }

    char uuid_str[BT_UUID_STR_LEN];
    bt_uuid_to_str(service->uuid, uuid_str, sizeof(uuid_str));
    debug_printk("Discovered primary service UUID: %s\n", uuid_str);

    if (bt_uuid_cmp(service->uuid, &led_svc_uuid.uuid) == 0) {
        led_svc_start_handle = attr->handle + 1U;
        led_svc_end_handle = service->end_handle;
        led_service_found = true;
        debug_printk("LED service handles: start %u end %u\n",
                     led_svc_start_handle, led_svc_end_handle);
        k_event_set(&event, EV_SERVICE_FOUND);
        return BT_GATT_ITER_STOP;
    }

    return BT_GATT_ITER_CONTINUE;
}

static uint8_t led_char_discover_cb(struct bt_conn *conn,
                                    const struct bt_gatt_attr *attr,
                                    struct bt_gatt_discover_params *prms) {
    ARG_UNUSED(conn);
    ARG_UNUSED(prms);
    if (!attr) {
        return BT_GATT_ITER_STOP;
    }
    led_state_handle = bt_gatt_attr_value_handle(attr);
    debug_printk("LED Control value handle %u\n", led_state_handle);
    k_event_set(&event, EV_HANDLE_FOUND);
    return BT_GATT_ITER_STOP;
}

static void discover_led_primary_service() {
    int err;

    led_service_found = false;
    led_svc_start_handle = 0;
    led_svc_end_handle = 0;

    svc_discover_params.uuid = NULL;
    svc_discover_params.func = led_primary_service_discover_cb;
    svc_discover_params.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
    svc_discover_params.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE;
    svc_discover_params.type = BT_GATT_DISCOVER_PRIMARY;

    err = bt_gatt_discover(led_conn, &svc_discover_params);
    if (err) {
        debug_printk("Primary service discover failed(err %d)\n", err);
        return;
    }
}

static void discover_led_service() {
    int err;

    discover_params.uuid = &led_state_char_uuid.uuid;
    discover_params.func = led_char_discover_cb;
    discover_params.start_handle = led_svc_start_handle ? led_svc_start_handle
                                                        : BT_ATT_FIRST_ATTRIBUTE_HANDLE;
    discover_params.end_handle = led_svc_end_handle ? led_svc_end_handle
                                                    : BT_ATT_LAST_ATTRIBUTE_HANDLE;
    discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;
  
    err = bt_gatt_discover(led_conn, &discover_params);
    if (err) {
        debug_printk("Discover failed(err %d)\n", err);
        return;
    }
}

static uint8_t read_func(struct bt_conn *conn, uint8_t err,
                         struct bt_gatt_read_params *params, const void *data,
                         uint16_t length) {
    ARG_UNUSED(conn);
    ARG_UNUSED(params);
    if (err) {
        debug_printk("Read failed: err %u\n", err);
        last_read_err = err;
        k_event_set(&event, EV_STATE_READ);
        return BT_GATT_ITER_STOP;
    }

    if (data == NULL) {
        if (last_read_err != 0) {
            debug_printk("Read completed without data\n");
        }
        k_event_set(&event, EV_STATE_READ);
        return BT_GATT_ITER_STOP;
    }

    if (length < sizeof(uint8_t)) {
        debug_printk("Read length too short: %u\n", length);
        last_read_err = -1;
        k_event_set(&event, EV_STATE_READ);
        return BT_GATT_ITER_STOP;
    }

    led_state = *((uint8_t *)data);
    last_read_err = 0;
    debug_printk("LED Control value: %u\n", led_state);
    k_event_set(&event, EV_STATE_READ);
    return BT_GATT_ITER_STOP;
}

static void write_func(struct bt_conn *conn, uint8_t err,
                       struct bt_gatt_write_params *params) {
    ARG_UNUSED(conn);
    ARG_UNUSED(params);
    if (err) {
        debug_printk("Write did not work: err %u\n", err);
        last_write_err = err;
    } else {
        debug_printk("Button press command sent successfully to peripheral\n");
        last_write_err = 0;
    }
    k_event_set(&event, EV_STATE_WRITTEN);
}

static void read_led_state() {
    debug_printk("Reading current LED state\n");
    static struct bt_gatt_read_params read_params;
    last_read_err = -1;
    read_params.handle_count = 1;
    read_params.single.handle = led_state_handle;
    read_params.single.offset = 0;
    read_params.func = read_func;
    bt_gatt_read(led_conn, &read_params);
}

static void write_led_state() {
    static struct bt_gatt_write_params write_params;
    led_state = 0xFF; /* Any non-zero value triggers state advancement */
    last_write_err = -1;
  
    debug_printk("Sending button press command to peripheral\n");
  
    write_params.handle = led_state_handle;
    write_params.offset = 0;
    write_params.data = &led_state;
    write_params.length = 1;
    write_params.func = write_func;
    bt_gatt_write(led_conn, &write_params);
}

static bool led_found_cb(struct bt_data *data, void *user_data) {
    struct bt_uuid_128 found_uuid;

    // Accept complete or partial 128-bit UUID lists
    if ((data->type != BT_DATA_UUID128_ALL) &&
        (data->type != BT_DATA_UUID128_SOME)) {
        return true;
    }

    if ((data->data_len % BT_UUID_SIZE_128) != 0) {
        return true;
    }

    for (uint8_t i = 0; i < data->data_len; i += BT_UUID_SIZE_128) {
        bt_uuid_create(&found_uuid.uuid, &data->data[i], BT_UUID_SIZE_128);
        if (bt_uuid_cmp(&found_uuid.uuid, &led_svc_uuid.uuid) == 0) {
            debug_printk("LED service found - UUID match confirmed\n");
            memcpy(&led_address, user_data, BT_ADDR_LE_SIZE);

            k_event_set(&event, EV_LED_FOUND);
            debug_printk("EV_LED_FOUND event set\n");

            stop_scan();
            return false;
        }
    }

    return true;
}

static void device_found_cb(const bt_addr_le_t *addr, int8_t rssi,
                            uint8_t type, struct net_buf_simple *ad) {
    ARG_UNUSED(rssi);
    ARG_UNUSED(type);
    bt_data_parse(ad, led_found_cb, (void *)addr);
}

void start_scan() {
    int err;

    struct bt_le_scan_param scan_param = {
        .type = BT_LE_SCAN_TYPE_ACTIVE,
        .options = BT_LE_SCAN_OPT_NONE,
        .interval = BT_GAP_SCAN_FAST_INTERVAL,
        .window = BT_GAP_SCAN_FAST_WINDOW,
    };

    err = bt_le_scan_start(&scan_param, device_found_cb);
    if (err) {
        debug_printk("Scanning failed to start (err %d)\n", err);
        return;
    }

    debug_printk("Scanning successfully started\n");
}

static void work_handler(struct k_work *work) {
    ARG_UNUSED(work);
    bool ok;

    if (led_conn == NULL) {
        k_event_clear(&event, EV_LED_FOUND | EV_CONNECTED | EV_HANDLE_FOUND |
                             EV_SERVICE_FOUND | EV_STATE_READ | EV_STATE_WRITTEN |
                             EV_DISCONNECTED);

        start_scan();
        debug_printk("Waiting for LED service to be found...\n");
        ok = wait_for_event(EV_LED_FOUND, EVENT_TIMEOUT_MS);
        if (!ok) {
            debug_printk("Rear Light not found - timeout waiting for LED service\n");
            stop_scan();
            k_sem_give(&sem);
            return;
        }

        debug_printk("LED found, proceeding to connect\n");
        k_event_clear(&event, EV_LED_FOUND);

        connect();
        ok = wait_for_event(EV_CONNECTED, EVENT_TIMEOUT_MS);
        if (!ok || led_conn == NULL) {
            debug_printk("Connection failed or timeout\n");
            k_sem_give(&sem);
            return;
        }

        k_event_clear(&event, EV_CONNECTED);
    }

    if (!led_state_handle) {
        k_event_clear(&event, EV_SERVICE_FOUND);
        k_msleep(100);
        discover_led_primary_service();
        ok = wait_for_event(EV_SERVICE_FOUND, DISCOVERY_TIMEOUT_MS);
        if (!ok || !led_service_found) {
            debug_printk("Primary service discovery failed or timeout\n");
            if (led_conn) {
                bt_conn_disconnect(led_conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
            }
            k_sem_give(&sem);
            return;
        }
        k_event_clear(&event, EV_SERVICE_FOUND);

        k_event_clear(&event, EV_HANDLE_FOUND);
        discover_led_service();
        ok = wait_for_event(EV_HANDLE_FOUND, DISCOVERY_TIMEOUT_MS);
        if (!ok && led_state_handle) {
            ok = true;
        }

        if (!ok || !led_state_handle) {
            debug_printk("Characteristic discovery failed or timeout\n");
            if (led_conn) {
                bt_conn_disconnect(led_conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
            }
            k_sem_give(&sem);
            return;
        }

        k_event_clear(&event, EV_HANDLE_FOUND);
    }

    k_event_clear(&event, EV_STATE_READ);
    read_led_state();
    ok = wait_for_event(EV_STATE_READ, EVENT_TIMEOUT_MS);
    if (!ok || last_read_err != 0) {
        debug_printk("Read failed or timeout\n");
        if (led_conn) {
            bt_conn_disconnect(led_conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
        }
        k_sem_give(&sem);
        return;
    }
    k_event_clear(&event, EV_STATE_READ);

    k_event_clear(&event, EV_STATE_WRITTEN);
    write_led_state();
    ok = wait_for_event(EV_STATE_WRITTEN, EVENT_TIMEOUT_MS);
    if (!ok || last_write_err != 0) {
        debug_printk("Write failed or timeout\n");
    }
    k_event_clear(&event, EV_STATE_WRITTEN);

    if (led_conn) {
        debug_printk("Disconnecting\n");
        bt_conn_disconnect(led_conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
        wait_for_event(EV_DISCONNECTED, EVENT_TIMEOUT_MS);
        k_event_clear(&event, EV_DISCONNECTED);
    }

    k_sem_give(&sem);
}

static void connected(struct bt_conn *conn, uint8_t conn_err) {
    ARG_UNUSED(conn);
    if (!conn_err) {
        debug_printk("Connected.\n");
        k_event_set(&event, EV_CONNECTED);
    } else {
        debug_printk("Failed to connect.\n");
        bt_conn_unref(led_conn);
        led_conn = NULL;
    }
  }

static void disconnected(struct bt_conn *conn, uint8_t reason) {
    ARG_UNUSED(conn);
    ARG_UNUSED(reason);	
    debug_printk("Disconnected.\n");
    bt_conn_unref(led_conn);
    led_conn = NULL;
    led_state_handle = 0;
    led_svc_start_handle = 0;
    led_svc_end_handle = 0;
    led_service_found = false;
    k_event_set(&event, EV_DISCONNECTED);

}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};

int bluetooth_init(void) {
    int err;
    err = bt_enable(NULL);
    if (err) {
        debug_printk("Bluetooth init failed (err %d)\n", err);
        return err;
    }
    debug_printk("Bluetooth initialized\n");
    k_work_init(&work, work_handler);
    return 0;
}

void ble_toogle_led() {
    if (k_sem_take(&sem, K_NO_WAIT) != 0) {
        return;
    };
  
    k_work_submit(&work);
}

#endif /* BLE_MASTER */
#endif /* BLUETOOTH */
