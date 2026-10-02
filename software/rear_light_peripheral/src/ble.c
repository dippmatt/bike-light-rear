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
#include <zephyr/sys/printk.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>

#include "ble.h"
#include "app_events.h"
#include "state_machine.h"
#include "debug.h"

/* --- Advertising --- */

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BLE_SERVICE_UUID)),
};

static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME,
            sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

static void adv_start_work(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(adv_work, adv_start_work);

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

/* --- GATT service --- */

static uint8_t led_status_value;
static uint8_t control_value;

static ssize_t read_led_status(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                               void *buf, uint16_t len, uint16_t offset)
{
    led_status_value = sm_current_state();

    return bt_gatt_attr_read(conn, attr, buf, len, offset, &led_status_value,
                             sizeof(led_status_value));
}

static ssize_t write_control(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                             const void *buf, uint16_t len, uint16_t offset,
                             uint8_t flags)
{
    ARG_UNUSED(conn);
    ARG_UNUSED(attr);
    ARG_UNUSED(flags);

    if (offset != 0U || len != sizeof(uint8_t)) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    control_value = ((const uint8_t *)buf)[0];

    if (control_value <= IDLE_CHARGING) {
        /* The state machine applies the stable-window guard and transition */
        app_event_post(EVT_BLE_SET_STATE, control_value);
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

int ble_init(void)
{
    int err;

    err = bt_enable(NULL);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return err;
    }

    printk("Bluetooth initialized\n");

    k_work_schedule(&adv_work, K_NO_WAIT);

    return 0;
}
