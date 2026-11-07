/*
 * ANT+ Bike Lights device profile, unconnected light. See bike_light.h.
 */

#include "bike_light.h"

#include "variant.h"

/* --- Device identity ------------------------------------------------- */

#define MANUFACTURER_ID   0x00FFU /* 255 = development ID (common page 80) */
/* MODEL_NUMBER depends on the variant (variant.h) */
#define HW_REVISION       1U
#define SW_REVISION_MAIN  1U
#define SW_REVISION_SUPP  0xFFU   /* invalid = not used */

#define BATTERY_CAPACITY_UNITS 5U /* 200 mAh units: 1100 mAh cell -> 5 (1000 mAh) */
#define SECONDARY_LIGHTS       4U /* profile minimum; advertised, shared channel not implemented */

/* ANT_LIGHT_TYPE (variant.h): Table 7-5 value, 0 headlight, 2 taillight */
#define LIGHT_TYPE_ANY          15U /* Table 7-41 "invalid" = all types */

#define BEAM_CMD_OFF 1U
#define BEAM_CMD_LOW 2U
#define BEAM_CMD_HIGH 3U

#define SEQ_NONE 0xFFU
#define EVENT_PAGE1_REPEATS 3U
#define MAX_REQUEST_REPEATS 4U

#define INTENSITY_STEADY_PERCENT LIGHT_BASE_PERCENT /* Base duty (LIGHT_PULSE_BASE) */
#define INTENSITY_BRIGHT_PERCENT LIGHT_PEAK_PERCENT /* Peak duty (LIGHT_PULSE_PEAK) */
#define INTENSITY_AUTO 0xFEU
#define INTENSITY_INVALID 0xFFU

/* --- Mode table: the single configuration point ---------------------- */

/*
 * Maps ANT+ standard mode numbers (Table 5-1) to this firmware's modes.
 * The first entry for a lamp mode is the one reported in page 1.
 * Supported modes in page 2 are derived from this table.
 * Entry 0 (Off) must stay first.
 */
static const struct {
    uint8_t ant_mode;
    enum lamp_mode lamp;
} mode_map[] = {
#if LIGHT_FRONT
    { 0U, LAMP_OFF },    /* Off */
    { 4U, LAMP_STEADY }, /* Steady 21-40% (Standard is a 30% duty) */
    { 2U, LAMP_BRIGHT }, /* Steady 61-80% (Bright is an 80% duty) */
    { 9U, LAMP_SMART },  /* Auto */
#else
    { 0U, LAMP_OFF },    /* Off */
    { 5U, LAMP_STEADY }, /* Steady 0-20% (Base is a 20% duty) */
    { 6U, LAMP_FLASH },  /* Slow flash */
    { 9U, LAMP_SMART },  /* Auto */
#endif
};

#define MODE_MAP_LEN (sizeof(mode_map) / sizeof(mode_map[0]))

uint8_t bike_light_ant_mode_of(enum lamp_mode mode)
{
    for (unsigned int i = 0; i < MODE_MAP_LEN; i++) {
        if (mode_map[i].lamp == mode) {
            return mode_map[i].ant_mode;
        }
    }
    return 0U;
}

bool bike_light_lamp_mode_of(uint8_t ant_mode, enum lamp_mode *mode)
{
    for (unsigned int i = 0; i < MODE_MAP_LEN; i++) {
        if (mode_map[i].ant_mode == ant_mode) {
            *mode = mode_map[i].lamp;
            return true;
        }
    }
    return false;
}

/* --- Controller side -------------------------------------------------- */

void bike_light_build_settings(uint8_t page[BL_PAGE_SIZE], uint8_t light_type, uint8_t seq,
                               uint8_t ant_mode, bool off)
{
    page[0] = BL_PAGE_SETTINGS;
    page[1] = BL_LIGHT_INDEX_UNCONNECTED; /* index 0: all lights (7.18.10) */
    page[2] = (uint8_t)((light_type << 4) | 0x08U); /* all sub-lights of that type */
    page[3] = seq;
    page[4] = 0x00U;
    page[5] = 0x00U;
    page[6] = off ? BEAM_CMD_OFF : (uint8_t)(ant_mode << 2);
    page[7] = 0xFFU;
}

/* --- Page builders --------------------------------------------------- */

static const uint8_t default_rotation[] = {
    BL_PAGE_LIGHT_STATE, BL_PAGE_CAPABILITIES, BL_PAGE_MANUFACTURER, BL_PAGE_PRODUCT,
    BL_PAGE_MAIN_CHANNEL_ID, /* only while connected */
};

#define ROTATION_LEN (sizeof(default_rotation) / sizeof(default_rotation[0]))

static bool is_connected(const struct bike_light *bl)
{
    return bl->light_index != BL_LIGHT_INDEX_UNCONNECTED;
}

/* The channel ID page is part of the rotation only in the connected state */
static unsigned int rotation_len(const struct bike_light *bl)
{
    return is_connected(bl) ? ROTATION_LEN : ROTATION_LEN - 1U;
}

static uint8_t intensity_of(enum lamp_mode mode)
{
    switch (mode) {
    case LAMP_OFF:
        return 0U;
    case LAMP_STEADY:
        return INTENSITY_STEADY_PERCENT;
    case LAMP_BRIGHT:
        return INTENSITY_BRIGHT_PERCENT;
    case LAMP_SMART:
        return INTENSITY_AUTO;
    case LAMP_FLASH:
    default:
        return INTENSITY_INVALID;
    }
}

static void build_light_state(const struct bike_light *bl, uint8_t p[BL_PAGE_SIZE])
{
    p[0] = BL_PAGE_LIGHT_STATE;
    p[1] = bl->light_index;
    /* bit 1 bike radar = 0, bits 2:4 light type, bits 5:7 battery status */
    p[2] = (uint8_t)((ANT_LIGHT_TYPE << 2) | ((bl->battery_status & 0x07U) << 5));
    p[3] = 0U; /* no sub-lights */
    p[4] = bl->cmd_seq;
    p[5] = 0xFFU; /* beam focus not supported */
    /* bits 0:1 beam (0 = low/normal), bits 2:7 current mode number */
    p[6] = (uint8_t)(bike_light_ant_mode_of(bl->mode) << 2);
    p[7] = intensity_of(bl->mode);
}

static void build_capabilities(const struct bike_light *bl, uint8_t p[BL_PAGE_SIZE])
{
    uint16_t modes = 0U;
    uint8_t mode_count = 0U;

    for (unsigned int i = 0; i < MODE_MAP_LEN; i++) {
        uint8_t m = mode_map[i].ant_mode;

        if (m >= 1U && m <= 14U) {
            modes |= (uint16_t)(1U << m);
            mode_count++;
        }
    }

    p[0] = BL_PAGE_CAPABILITIES;
    p[1] = bl->light_index;
    /* bit 0 auto intensity setting: no, bit 1 high/low beam: no,
     * bits 2:7 total number of modes = standard modes in the bit field below
     * plus custom modes (none). Bit 0 of the bit field (Off) is reserved and
     * stays clear, so Off is not counted: a controller that finds fewer
     * standard modes than this count assumes the rest are custom modes 63,
     * 62, ... (section 5.3) and commands mode 63 (seen on a Wahoo ELEMNT). */
    p[2] = (uint8_t)(mode_count << 2);
    p[3] = BATTERY_CAPACITY_UNITS;
    /* bits 0:3 secondary lights, bit 6 beam focus control: 1 = incapable,
     * bit 7 beam intensity control: 0 = incapable (table 7-8) */
    p[4] = (uint8_t)(SECONDARY_LIGHTS | (1U << 6));
    p[5] = (uint8_t)(modes & 0xFFU);
    p[6] = (uint8_t)((modes >> 8) & 0x7FU); /* bit 7 synchronous brake: no */
    p[7] = (uint8_t)(1U << ANT_LIGHT_TYPE); /* supported light types: this one */
}

static void build_supplementary(const struct bike_light *bl, uint8_t p[BL_PAGE_SIZE])
{
    p[0] = BL_PAGE_SUPPLEMENTARY;
    p[1] = bl->light_index;
    p[2] = 0U; /* sub-light index 0 */
    p[3] = 0U;
    p[4] = 0U;
    p[5] = 0U;
    p[6] = 0U;
    p[7] = bl->battery_percent;
}

static void build_manufacturer(uint8_t p[BL_PAGE_SIZE])
{
    /* Common page 80: reserved, reserved, HW rev, mfg ID (LE), model (LE) */
    p[0] = BL_PAGE_MANUFACTURER;
    p[1] = 0xFFU;
    p[2] = 0xFFU;
    p[3] = HW_REVISION;
    p[4] = (uint8_t)(MANUFACTURER_ID & 0xFFU);
    p[5] = (uint8_t)(MANUFACTURER_ID >> 8);
    p[6] = (uint8_t)(MODEL_NUMBER & 0xFFU);
    p[7] = (uint8_t)(MODEL_NUMBER >> 8);
}

static void build_product(const struct bike_light *bl, uint8_t p[BL_PAGE_SIZE])
{
    /* Common page 81: reserved, SW rev supplemental, SW rev main, serial (LE) */
    p[0] = BL_PAGE_PRODUCT;
    p[1] = 0xFFU;
    p[2] = SW_REVISION_SUPP;
    p[3] = SW_REVISION_MAIN;
    p[4] = (uint8_t)(bl->serial & 0xFFU);
    p[5] = (uint8_t)((bl->serial >> 8) & 0xFFU);
    p[6] = (uint8_t)((bl->serial >> 16) & 0xFFU);
    p[7] = (uint8_t)((bl->serial >> 24) & 0xFFU);
}

static bool build_main_channel_id(const struct bike_light *bl, uint8_t p[BL_PAGE_SIZE])
{
    uint16_t device_number = bike_light_device_number(bl->serial);

    if (!is_connected(bl)) {
        return false; /* not transmitted while unconnected (7.16.1) */
    }
    p[0] = BL_PAGE_MAIN_CHANNEL_ID;
    p[1] = 0xFFU;
    p[2] = 0xFFU;
    p[3] = 0xFFU;
    p[4] = 0xFFU;
    p[5] = (uint8_t)(device_number & 0xFFU);
    p[6] = (uint8_t)(device_number >> 8);
    p[7] = BL_TRANSMISSION_TYPE;
    return true;
}

static bool build_page(const struct bike_light *bl, uint8_t page, uint8_t p[BL_PAGE_SIZE])
{
    switch (page) {
    case BL_PAGE_LIGHT_STATE:
        build_light_state(bl, p);
        return true;
    case BL_PAGE_CAPABILITIES:
        build_capabilities(bl, p);
        return true;
    case BL_PAGE_SUPPLEMENTARY:
        build_supplementary(bl, p);
        return true;
    case BL_PAGE_MANUFACTURER:
        build_manufacturer(p);
        return true;
    case BL_PAGE_PRODUCT:
        build_product(bl, p);
        return true;
    case BL_PAGE_MAIN_CHANNEL_ID:
        return build_main_channel_id(bl, p);
    default:
        return false;
    }
}

/* --- State ----------------------------------------------------------- */

static void start_event(struct bike_light *bl)
{
    /* Pattern restarts after an event: page 1 three times, then 2, 80, 81 */
    bl->event_repeats = EVENT_PAGE1_REPEATS;
    bl->rotation_index = 1U;
}

uint16_t bike_light_device_number(uint32_t serial)
{
    uint16_t number = (uint16_t)(serial & 0xFFFFU);

    return (number == 0U) ? 1U : number;
}

void bike_light_init(struct bike_light *bl, uint32_t serial)
{
    bl->mode = LAMP_OFF;
    bl->last_on_mode = LAMP_STEADY;
    bl->cmd_seq = SEQ_NONE;
    bl->battery_status = BL_BATT_INVALID;
    bl->battery_percent = 0xFFU;
    bl->serial = serial;
    bl->light_index = BL_LIGHT_INDEX_UNCONNECTED;
    bl->event_repeats = 0U;
    bl->rotation_index = 0U;
    bl->request_page = 0U;
    bl->request_repeats = 0U;
}

void bike_light_set_mode(struct bike_light *bl, enum lamp_mode mode)
{
    if (mode >= LAMP_MODE_COUNT) {
        return;
    }
    if (mode != LAMP_OFF) {
        bl->last_on_mode = (uint8_t)mode;
    }
    if (mode != bl->mode) {
        bl->mode = mode;
        start_event(bl);
    }
}

void bike_light_disconnect(struct bike_light *bl)
{
    if (is_connected(bl)) {
        bl->light_index = BL_LIGHT_INDEX_UNCONNECTED;
        start_event(bl);
    }
}

bool bike_light_is_connected(const struct bike_light *bl)
{
    return is_connected(bl);
}

void bike_light_set_battery(struct bike_light *bl, uint8_t status, uint8_t percent)
{
    bool changed = (status != bl->battery_status);

    bl->battery_status = status;
    bl->battery_percent = percent;
    if (changed) {
        start_event(bl);
    }
}

void bike_light_next_page(struct bike_light *bl, uint8_t page[BL_PAGE_SIZE])
{
    if (bl->event_repeats > 0U) {
        bl->event_repeats--;
        build_light_state(bl, page);
        return;
    }

    if (bl->request_repeats > 0U) {
        bl->request_repeats--;
        if (build_page(bl, bl->request_page, page)) {
            return;
        }
    }

    (void)build_page(bl, default_rotation[bl->rotation_index % rotation_len(bl)], page);
    bl->rotation_index = (uint8_t)((bl->rotation_index + 1U) % rotation_len(bl));
}

/* --- Received pages --------------------------------------------------- */

/* Decision tree of profile 7.18.10 for a light without sub-lights. Light
 * index 0 addresses all lights, otherwise only the light with that index:
 * once connected this light is the main light, index 1. */
static bool settings_apply_to_us(const struct bike_light *bl, const uint8_t d[BL_PAGE_SIZE])
{
    uint8_t light_index = d[1];
    uint8_t sub_index = d[2] & 0x07U;
    bool all_sublights = ((d[2] >> 3) & 0x01U) != 0U;
    uint8_t light_type = (uint8_t)(d[2] >> 4);

    if (light_index != BL_LIGHT_INDEX_UNCONNECTED) {
        /* addressed to one light: must be us, and we must be connected */
        return is_connected(bl) && light_index == bl->light_index &&
               (sub_index == 0U || all_sublights);
    }
    if (all_sublights) {
        return light_type == LIGHT_TYPE_ANY || light_type == ANT_LIGHT_TYPE;
    }
    return sub_index == 0U;
}

static bool handle_settings(struct bike_light *bl, const uint8_t d[BL_PAGE_SIZE],
                            enum lamp_mode *requested)
{
    uint8_t beam;
    uint8_t ant_mode;

    /* The sequence number is updated for every command (tree start) */
    bl->cmd_seq = d[3];
    start_event(bl);

    if (!settings_apply_to_us(bl, d)) {
        return false;
    }

    beam = d[6] & 0x03U;
    ant_mode = (uint8_t)(d[6] >> 2);

    if (beam == BEAM_CMD_OFF) {
        *requested = LAMP_OFF;
        return true;
    }
    if (ant_mode != 0U) {
        /* unsupported modes are ignored (7.18.9) */
        return bike_light_lamp_mode_of(ant_mode, requested);
    }
    if ((beam == BEAM_CMD_LOW || beam == BEAM_CMD_HIGH) && bl->mode == LAMP_OFF) {
        *requested = (enum lamp_mode)bl->last_on_mode;
        return true;
    }
    return false;
}

/* Page 33, light state setting (Table 7-36) */
static bool connect_state_request(uint8_t state, enum lamp_mode *requested)
{
    switch (state) {
    case 1U: /* off */
        *requested = LAMP_OFF;
        return true;
    case 2U: /* flashing: any mode without constant intensity */
        *requested = LAMP_FLASH;
        return true;
    case 3U: /* steady: any mode with constant intensity */
        *requested = LAMP_STEADY;
        return true;
    default: /* 0 = do not change */
        return false;
    }
}

/* Page 33 (7.17). A controller uses it to form a network, even when there
 * is only one light: this light becomes the main light, index 1. The
 * light type setting is ignored; this light reports its own type in
 * page 1 (7.17.6: unsupported types are ignored). */
static bool handle_connect(struct bike_light *bl, const uint8_t d[BL_PAGE_SIZE],
                           enum lamp_mode *requested)
{
    uint8_t index_setting = d[1];
    uint8_t controller_id = d[3];
    uint8_t sub_index = d[4] & 0x07U;
    uint8_t state = (d[4] >> 3) & 0x03U;

    if (!is_connected(bl)) {
        if (index_setting == BL_LIGHT_INDEX_MAIN) {
            bl->light_index = BL_LIGHT_INDEX_MAIN;
        } else if (index_setting != 0xFFU) {
            /* secondary light (2-63) needs the shared slave channel: not
             * implemented; reserved values are ignored */
            return false;
        }
    } else if (index_setting != BL_LIGHT_INDEX_MAIN) {
        return false; /* not for the main light; no secondary lights to forward to */
    }

    bl->cmd_seq = controller_id; /* echoed in page 1 (7.4.7) */
    start_event(bl);

    return sub_index == 0U && connect_state_request(state, requested);
}

/* Page 32 (7.16): back to the unconnected state */
static void handle_disconnect(struct bike_light *bl, const uint8_t d[BL_PAGE_SIZE])
{
    uint8_t index = d[1];

    if (is_connected(bl) && index != 0U && index != bl->light_index) {
        return; /* addressed to a secondary light */
    }
    bl->light_index = BL_LIGHT_INDEX_UNCONNECTED;
    bl->cmd_seq = d[2];
    start_event(bl);
}

static void handle_request(struct bike_light *bl, const uint8_t d[BL_PAGE_SIZE])
{
    uint8_t page = d[6];
    uint8_t repeats = d[5] & 0x7FU;
    uint8_t scratch[BL_PAGE_SIZE];

    /* VERIFY: byte 1 is the light index in this profile but the serial number
     * LSB in generic common page 70 use, so it is not checked. */
    if (d[7] != 0x01U) {
        return; /* not a "request data page" command */
    }
    if (repeats == 0U) {
        if ((d[5] & 0x80U) == 0U) {
            return; /* 0x00 = invalid */
        }
        repeats = MAX_REQUEST_REPEATS; /* 0x80 = until acknowledged */
    }
    if (repeats > MAX_REQUEST_REPEATS) {
        repeats = MAX_REQUEST_REPEATS;
    }
    if (!build_page(bl, page, scratch)) {
        return; /* page not supported: ignore */
    }

    bl->request_page = page;
    bl->request_repeats = repeats;
}

bool bike_light_handle_rx(struct bike_light *bl, const uint8_t payload[BL_PAGE_SIZE],
                          enum lamp_mode *requested)
{
    switch (payload[0]) {
    case BL_PAGE_SETTINGS:
        return handle_settings(bl, payload, requested);
    case BL_PAGE_REQUEST:
        handle_request(bl, payload);
        return false;
    case BL_PAGE_DISCONNECT:
        handle_disconnect(bl, payload);
        return false;
    case BL_PAGE_CONNECT:
        return handle_connect(bl, payload, requested);
    default:
        return false; /* undefined pages are handled gracefully (8.3) */
    }
}
