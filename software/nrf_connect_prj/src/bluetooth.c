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
#include "utils.h"

#ifndef BLE_MASTER
/* ========== SLAVE MODE IMPLEMENTATION ========== */

/* UUID definitions for slave mode */
static struct bt_uuid_128 button_service_uuid = BT_UUID_INIT_128(BT_UUID_BUTTON_SERVICE_VAL);
static struct bt_uuid_128 button_press_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef1));

/* Button press characteristic value */
static uint8_t button_press_value = 0;

/**
 * @brief Write callback for button press characteristic
 */
static ssize_t write_button_press(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                   const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
    ARG_UNUSED(conn);
    ARG_UNUSED(attr);
    ARG_UNUSED(flags);
    
    if (offset + len > sizeof(button_press_value)) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }
    
    if (len != sizeof(button_press_value)) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }
    
    uint8_t value = *((const uint8_t *)buf);
    
    if (value == 1) {
        /* Trigger button press - advance state machine */
        debug_printk("BLE: Button press received from master\n");
        main_state_machine_on_button_press();
    }
    
    button_press_value = value;
    
    return len;
}

/* GATT Service Definition */
BT_GATT_SERVICE_DEFINE(button_svc,
    BT_GATT_PRIMARY_SERVICE(&button_service_uuid),
    BT_GATT_CHARACTERISTIC(&button_press_uuid.uuid,
                           BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_WRITE,
                           NULL, write_button_press, &button_press_value),
);

/**
 * @brief Connection callback
 */
static void connected(struct bt_conn *conn, uint8_t err)
{
    char addr[BT_ADDR_LE_STR_LEN];
    
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    
    if (err) {
        debug_printk("BLE: Connection failed (err %u)\n", err);
    } else {
        debug_printk("BLE: Connected: %s\n", addr);
    }
}

/**
 * @brief Disconnection callback
 */
static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    char addr[BT_ADDR_LE_STR_LEN];
    
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    debug_printk("BLE: Disconnected: %s (reason %u)\n", addr, reason);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};

/* Advertising data with service UUID */
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_BUTTON_SERVICE_VAL),
};

/**
 * @brief Initialize BLE in slave mode
 */
int bluetooth_init(void)
{
    int err;
    
    err = bt_enable(NULL);
    if (err) {
        debug_printk("BLE: Enable failed (err %d)\n", err);
        return err;
    }
    
    debug_printk("BLE: Bluetooth initialized (slave mode)\n");
    
    /* Start advertising */
    err = bt_le_adv_start(
        BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONN,
                        BT_GAP_ADV_FAST_INT_MIN_2,
                        BT_GAP_ADV_FAST_INT_MAX_2,
                        NULL),
        ad, ARRAY_SIZE(ad), NULL, 0);
    
    if (err) {
        debug_printk("BLE: Advertising failed to start (err %d)\n", err);
        return err;
    }
    
    debug_printk("BLE: Advertising started\n");
    
    return 0;
}

#else
/* ========== MASTER MODE IMPLEMENTATION ========== */

#include <bluetooth/scan.h>

/* UUID definitions for master mode */
static struct bt_uuid_128 button_press_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef1));

/* Connection state */
static struct bt_conn *active_conn = NULL;
static volatile bool connection_established = false;
static uint16_t button_press_handle = 0;
static bool handle_discovered = false;

/**
 * @brief Connection callback
 */
static void connected(struct bt_conn *conn, uint8_t err)
{
    char addr[BT_ADDR_LE_STR_LEN];
    
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    
    if (err) {
        debug_printk("BLE Master: Connection failed to %s (err %u)\n", addr, err);
        connection_established = false;
    } else {
        debug_printk("BLE Master: Connected: %s\n", addr);
        active_conn = bt_conn_ref(conn);
        connection_established = true;
        handle_discovered = false;
        button_press_handle = 0;
    }
}

/**
 * @brief Disconnection callback
 */
static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    char addr[BT_ADDR_LE_STR_LEN];
    
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    debug_printk("BLE Master: Disconnected: %s (reason %u)\n", addr, reason);
    
    if (conn == active_conn) {
        bt_conn_unref(active_conn);
        active_conn = NULL;
    }
    
    connection_established = false;
    handle_discovered = false;
    button_press_handle = 0;
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};

/**
 * @brief Scan filter match callback
 */
static void scan_filter_match(struct bt_scan_device_info *device_info,
                               struct bt_scan_filter_match *filter_match, bool connectable)
{
    ARG_UNUSED(filter_match);
    
    char addr[BT_ADDR_LE_STR_LEN];
    
    bt_addr_le_to_str(device_info->recv_info->addr, addr, sizeof(addr));
    debug_printk("BLE Master: Filters matched. Address: %s connectable: %d\n", addr, connectable);
}

/**
 * @brief Scan connecting error callback
 */
static void scan_connecting_error(struct bt_scan_device_info *device_info)
{
    ARG_UNUSED(device_info);
    
    debug_printk("BLE Master: Connecting failed\n");
}

BT_SCAN_CB_INIT(scan_cb, scan_filter_match, NULL, scan_connecting_error, NULL);

/**
 * @brief GATT discovery callback
 */
static uint8_t discover_func(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                              struct bt_gatt_discover_params *params)
{
    ARG_UNUSED(conn);
    
    if (!attr) {
        debug_printk("BLE Master: Discovery complete\n");
        (void)memset(params, 0, sizeof(*params));
        return BT_GATT_ITER_STOP;
    }
    
    if (params->type == BT_GATT_DISCOVER_CHARACTERISTIC) {
        const struct bt_gatt_chrc *chrc = (const struct bt_gatt_chrc *)attr->user_data;
        
        if (bt_uuid_cmp(chrc->uuid, &button_press_uuid.uuid) == 0) {
            button_press_handle = chrc->value_handle;
            handle_discovered = true;
            debug_printk("BLE Master: Found button press handle: %u\n", button_press_handle);
            (void)memset(params, 0, sizeof(*params));
            return BT_GATT_ITER_STOP;
        }
    }
    
    return BT_GATT_ITER_CONTINUE;
}

/**
 * @brief Discover button service and characteristic
 */
static int discover_button_service(void)
{
    if (handle_discovered) {
        return 0;
    }
    
    static struct bt_gatt_discover_params discover_params;
    
    handle_discovered = false;
    discover_params.uuid = &button_press_uuid.uuid;
    discover_params.func = discover_func;
    discover_params.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
    discover_params.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE;
    discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;
    
    int err = bt_gatt_discover(active_conn, &discover_params);
    if (err) {
        debug_printk("BLE Master: Characteristic discovery failed (err %d)\n", err);
        return err;
    }
    
    /* Wait for discovery by polling (like Nordic example) */
    uint32_t timeout = k_uptime_get_32() + 5000;
    while (!handle_discovered && k_uptime_get_32() < timeout) {
        k_yield();
    }
    
    if (!handle_discovered) {
        debug_printk("BLE Master: Characteristic discovery timeout\n");
        return -ETIMEDOUT;
    }
    
    return 0;
}

/**
 * @brief Start scanning for slave devices
 */
static void scan_start(void)
{
    int err;
    
    struct bt_le_scan_param scan_param = {
        .type = BT_LE_SCAN_TYPE_PASSIVE,
        .options = BT_LE_SCAN_OPT_FILTER_DUPLICATE,
        .interval = BT_GAP_SCAN_FAST_INTERVAL,
        .window = BT_GAP_SCAN_FAST_WINDOW,
    };
    
    struct bt_le_conn_param *conn_param =
        BT_LE_CONN_PARAM(BT_GAP_INIT_CONN_INT_MIN, BT_GAP_INIT_CONN_INT_MAX, 0, 400);
    
    struct bt_scan_init_param scan_init = {
        .connect_if_match = true,
        .scan_param = &scan_param,
        .conn_param = conn_param,
    };
    
    bt_scan_init(&scan_init);
    bt_scan_cb_register(&scan_cb);
    
    err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_UUID,
                             BT_UUID_DECLARE_128(BT_UUID_BUTTON_SERVICE_VAL));
    if (err) {
        debug_printk("BLE Master: Scan filter add failed (err %d)\n", err);
        return;
    }
    
    err = bt_scan_filter_enable(BT_SCAN_UUID_FILTER, false);
    if (err) {
        debug_printk("BLE Master: Scan filter enable failed (err %d)\n", err);
        return;
    }
    
    err = bt_scan_start(BT_SCAN_TYPE_SCAN_PASSIVE);
    if (err) {
        debug_printk("BLE Master: Scan start failed (err %d)\n", err);
        return;
    }
    
    debug_printk("BLE Master: Scanning started\n");
}

/**
 * @brief Initialize BLE in master mode
 */
int bluetooth_init(void)
{
    int err;
    
    err = bt_enable(NULL);
    if (err) {
        debug_printk("BLE Master: Enable failed (err %d)\n", err);
        return err;
    }
    
    debug_printk("BLE Master: Bluetooth initialized (master mode)\n");
    
    /* Start scanning */
    scan_start();
    
    /* Wait for connection (like Nordic example) */
    while (true) {
        if (connection_established) {
            debug_printk("BLE Master: Connection established\n");
            break;
        }
        k_yield();
    }
    
    /* Discover service */
    err = discover_button_service();
    if (err) {
        debug_printk("BLE Master: Service discovery failed (err %d)\n", err);
        return err;
    }
    
    debug_printk("BLE Master: Ready to send button presses\n");
    
    return 0;
}

/**
 * @brief Send button press to slave
 */
int bluetooth_master_send_button_press(void)
{
    if (!active_conn || !handle_discovered) {
        return -ENOTCONN;
    }
    
    uint8_t value = 1;
    
    int err = bt_gatt_write_without_response(active_conn, button_press_handle, &value, sizeof(value), false);
    if (err) {
        debug_printk("BLE Master: Write failed (err %d)\n", err);
        return err;
    }
    
    debug_printk("BLE Master: Button press sent to slave\n");
    return 0;
}

#endif /* BLE_MASTER */

