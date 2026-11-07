/*
 * Debug counters readable over SWD without a console:
 *   make stats
 */

#ifndef STATS_H
#define STATS_H

#include <stdint.h>

#define STATS_MAGIC 0x414E5450UL /* "ANTP" */
#define STATS_RX_LOG_LEN 8U      /* received messages kept, newest overwrites oldest */

/* One received ANT message: ID (0x4E broadcast, 0x4F acknowledged) and the
 * 8 data bytes. `seq` counts messages since boot so the order can be read. */
struct rx_log_entry {
    uint32_t seq;
    uint8_t msg_id;
    uint8_t reserved[3];
    uint8_t data[8];
};

struct proto_stats {
    uint32_t magic;
    uint32_t uptime_s;       /* seconds since boot, updated by the battery tick */
    uint32_t tx_events;      /* EVENT_TX: one per channel period (~8.02 Hz) */
    uint32_t rx_messages;    /* messages received from the controller */
    uint32_t rx_commands;    /* page 34 commands decoded */
    uint32_t rx_requests;    /* page 70 requests decoded */
    uint32_t mode_changes;   /* mode changes applied (remote or button) */
    uint32_t battery_mv;
    uint32_t last_error;     /* last non-zero SoftDevice return code */
    uint32_t last_error_site;/* which call returned it */
    uint32_t sd_fault_id;
    uint32_t sd_fault_pc;
    uint32_t sd_fault_info;
    uint8_t last_rx[8];      /* last payload received from the controller */
    uint8_t last_tx[8];      /* last page queued for transmission */
    uint8_t mode;            /* current enum lamp_mode */
    uint8_t light_index;     /* 0 unconnected, 1 connected as main light */
    uint8_t reserved[2];
    /* ANT channel events by code (EVENT_TX is 3 ... 0x3F); index 0x40 counts
     * EVENT_RX (0x80), index 0x41 any other code */
    uint16_t event_count[0x42];
    uint32_t rx_other;       /* RX messages that are neither broadcast nor acknowledged */
    uint32_t rx_other_id;    /* ID of the last of those */
    uint32_t rx_log_next;    /* sequence number of the next log entry */
    struct rx_log_entry rx_log[STATS_RX_LOG_LEN];

    /* Application state (SPEC ANT-15) */
    uint32_t events_dropped;   /* event queue overflows (SYS-4) */
    uint32_t reset_reason;     /* RESETREAS at boot, 0 = power-on */
    uint32_t ant_init_error;   /* SoftDevice enable error, 0 = running with ANT */
    uint32_t silence_timeouts; /* connected -> unconnected because the controller went quiet */
    uint8_t state;             /* enum system_state */
    uint8_t sleeping;          /* 1 in Deep Sleep */
    uint8_t usb;               /* 1 with USB power */
    uint8_t low_battery;
    uint8_t accel_ok;          /* accelerometer initialised */
    uint8_t light_ok;          /* ambient light sensor initialised */
    uint8_t channel_state;     /* enum ant_channel_state */
    uint8_t parked;            /* 1 while Smart is parked (SMART-16) */

    /* Last sensor values and counters of the Smart sensors */
    int32_t accel_milli[3];    /* milli-m/s^2 */
    int32_t millilux;          /* last ambient light reading */
    uint32_t blank_count;      /* ambient measurements with the LED blanked */

    /* Bench builds only (make BENCH=1): write (arg << 8 | type) + 1 << 16 here
     * over SWD to post an event, see tools/inject.py */
    uint32_t inject;
};

extern volatile struct proto_stats proto_stats;

#endif /* STATS_H */
