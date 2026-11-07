/*
 * ANT+ Bike Lights device profile (D00001621 Rev 2.0), unconnected light.
 *
 * Pure protocol logic: no SoftDevice, no hardware. It builds the 8-byte
 * data pages the light broadcasts and decodes the 8-byte pages it
 * receives from the controller (bike computer). This keeps it testable on
 * the host (see test/test_bike_light.c).
 *
 * Scope of the bench prototype (see README.md):
 *   - unconnected state (light index 0) and the connected state as the only
 *     light of a network (light index 1, "main light" without secondary
 *     lights: the shared channel is not opened because nobody joins it).
 *     A controller such as the Wahoo ELEMNT forms this network with the
 *     connect command (page 33) before it sends any light settings.
 *   - pages sent: 1, 2, 80, 81, 18 (connected), 19 (on request)
 *   - pages received: 32 (disconnect), 33 (connect), 34 (light settings),
 *     70 (request page)
 *
 * Page layouts follow D00001621 Rev 2.0_M.001. Anything not specified there
 * and found on a real controller is marked "VERIFY".
 */

#ifndef BIKE_LIGHT_H
#define BIKE_LIGHT_H

#include <stdbool.h>
#include <stdint.h>

#define BL_PAGE_SIZE 8U

/* Channel transmission type: lower nibble 5 (independent channel, global
 * pages), upper nibble 0 = extended device number not used (profile 6.3.1) */
#define BL_TRANSMISSION_TYPE 0x05U

#define BL_LIGHT_INDEX_UNCONNECTED 0U
#define BL_LIGHT_INDEX_MAIN 1U

/* Light modes of this firmware as the ANT+ side sees them; main.c maps
 * them to the system states */
enum lamp_mode {
    LAMP_OFF = 0,
    LAMP_STEADY = 1,
    LAMP_FLASH = 2,
    LAMP_SMART = 3,
    LAMP_BRIGHT = 4, /* front light */
    LAMP_MODE_COUNT
};

/* ANT+ battery status values (profile Table 7-6) */
enum bl_battery_status {
    BL_BATT_NEW = 1,
    BL_BATT_GOOD = 2,
    BL_BATT_OK = 3,
    BL_BATT_LOW = 4,
    BL_BATT_CRITICAL = 5,
    BL_BATT_CHARGING = 6,
    BL_BATT_INVALID = 7,
};

/* Data page numbers */
#define BL_PAGE_LIGHT_STATE 0x01U
#define BL_PAGE_CAPABILITIES 0x02U
#define BL_PAGE_MAIN_CHANNEL_ID 0x12U
#define BL_PAGE_SUPPLEMENTARY 0x13U
#define BL_PAGE_DISCONNECT 0x20U
#define BL_PAGE_CONNECT 0x21U
#define BL_PAGE_SETTINGS 0x22U
#define BL_PAGE_REQUEST 0x46U
#define BL_PAGE_MANUFACTURER 0x50U
#define BL_PAGE_PRODUCT 0x51U

struct bike_light {
    enum lamp_mode mode;       /* current mode, reported in page 1 */
    uint8_t last_on_mode;      /* lamp_mode to resume on "beam on" commands */
    uint8_t cmd_seq;           /* sequence number of last command (0xFF = none) */
    uint8_t battery_status;    /* enum bl_battery_status */
    uint8_t battery_percent;   /* 0..100, 0xFF = invalid */
    uint32_t serial;           /* common page 81 serial number */
    uint8_t light_index;       /* 0 unconnected, 1 main light of a network */

    uint8_t event_repeats;     /* page 1 repetitions still to send */
    uint8_t rotation_index;    /* next page of the default rotation */
    uint8_t request_page;      /* page requested by the controller */
    uint8_t request_repeats;   /* repetitions of the requested page left */
};

void bike_light_init(struct bike_light *bl, uint32_t serial);

/** ANT channel device number for a serial: low 16 bits, never 0 */
uint16_t bike_light_device_number(uint32_t serial);

/**
 * Report a mode change (local button, command applied, ...).
 * Restarts the transmission pattern with page 1 (profile 7.3.2).
 */
void bike_light_set_mode(struct bike_light *bl, enum lamp_mode mode);

/**
 * The controller went quiet: return to the unconnected state (light index 0)
 * and restart the pattern. The light's mode is not touched (ANT-11).
 */
void bike_light_disconnect(struct bike_light *bl);

bool bike_light_is_connected(const struct bike_light *bl);

void bike_light_set_battery(struct bike_light *bl, uint8_t status, uint8_t percent);

/**
 * Fill the next page of the transmission pattern.
 * Priority: page 1 after an event, requested page, default rotation.
 */
void bike_light_next_page(struct bike_light *bl, uint8_t page[BL_PAGE_SIZE]);

/**
 * Decode a page received from the controller.
 *
 * @param[out] requested set when a mode change is commanded
 * @return true if the controller requested a mode change; the caller
 *         applies it (or not) and then reports the real result with
 *         bike_light_set_mode()
 */
bool bike_light_handle_rx(struct bike_light *bl, const uint8_t payload[BL_PAGE_SIZE],
                          enum lamp_mode *requested);

/* Mode table helpers (single configuration point, see mode_map in the .c) */
uint8_t bike_light_ant_mode_of(enum lamp_mode mode);
bool bike_light_lamp_mode_of(uint8_t ant_mode, enum lamp_mode *mode);

#endif /* BIKE_LIGHT_H */
