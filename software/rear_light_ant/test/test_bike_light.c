/*
 * Host unit tests for the ANT+ Bike Lights profile logic.
 * Build and run with: make test
 */

#include <stdio.h>
#include <string.h>

#include "../src/bike_light.h"

static int failures;

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);           \
            failures++;                                                      \
        }                                                                    \
    } while (0)

static void make_settings(uint8_t d[8], uint8_t index, uint8_t sub_byte, uint8_t seq,
                          uint8_t beam, uint8_t ant_mode)
{
    memset(d, 0, 8);
    d[0] = BL_PAGE_SETTINGS;
    d[1] = index;
    d[2] = sub_byte;
    d[3] = seq;
    d[4] = 0x42;
    d[5] = 0x03 | 0x0C | 0x30; /* invalid / no change for auto, brake, beam */
    d[6] = (uint8_t)((ant_mode << 2) | beam);
    d[7] = 0xFF;
}

static void test_default_rotation(void)
{
    struct bike_light bl;
    uint8_t p[8];
    const uint8_t expected[] = { 0x01, 0x02, 0x50, 0x51, 0x01, 0x02, 0x50, 0x51 };

    bike_light_init(&bl, 0x11223344U);
    for (unsigned int i = 0; i < sizeof(expected); i++) {
        bike_light_next_page(&bl, p);
        CHECK(p[0] == expected[i]);
    }
}

static void test_event_restarts_pattern(void)
{
    struct bike_light bl;
    uint8_t p[8];
    const uint8_t expected[] = { 0x01, 0x01, 0x01, 0x02, 0x50, 0x51, 0x01 };

    bike_light_init(&bl, 1U);
    bike_light_next_page(&bl, p);
    bike_light_next_page(&bl, p);
    bike_light_set_mode(&bl, LAMP_FLASH);
    for (unsigned int i = 0; i < sizeof(expected); i++) {
        bike_light_next_page(&bl, p);
        CHECK(p[0] == expected[i]);
    }
}

static void test_page1_content(void)
{
    struct bike_light bl;
    uint8_t p[8];

    bike_light_init(&bl, 1U);
    bike_light_set_battery(&bl, BL_BATT_GOOD, 77U);
    bike_light_set_mode(&bl, LAMP_FLASH);
    bike_light_next_page(&bl, p);

    CHECK(p[0] == 0x01);
    CHECK(p[1] == 0x00);                   /* unconnected */
    CHECK(((p[2] >> 2) & 0x07) == 2);      /* taillight */
    CHECK(((p[2] >> 5) & 0x07) == BL_BATT_GOOD);
    CHECK((p[3] & 0x07) == 0);             /* no sub-lights */
    CHECK(p[4] == 0xFF);                   /* no command yet */
    CHECK(p[5] == 0xFF);                   /* beam focus invalid */
    CHECK((p[6] >> 2) == 6);               /* slow flash */
    CHECK((p[6] & 0x03) == 0);             /* low beam */
}

static void test_capabilities(void)
{
    struct bike_light bl;
    uint8_t p[8];

    bike_light_init(&bl, 1U);
    bike_light_set_mode(&bl, LAMP_OFF);
    /* skip to page 2 */
    bike_light_next_page(&bl, p);
    bike_light_next_page(&bl, p);
    CHECK(p[0] == 0x02);
    CHECK((p[2] >> 2) == 3);          /* steady + flash + auto, no custom modes */
    CHECK((p[5] & (1U << 5)) != 0);   /* mode 5 */
    CHECK((p[5] & (1U << 6)) != 0);   /* mode 6 */
    CHECK((p[5] & (1U << 1)) == 0);   /* mode 1 not offered */
    CHECK((p[6] & (1U << 1)) != 0);   /* mode 9 = bit 1 of MSB */
    CHECK((p[4] & 0x0FU) == 4);       /* secondary lights */
    CHECK((p[4] & 0xC0U) == 0x40U);   /* beam focus incapable, intensity incapable (table 7-8) */
    CHECK((p[7] & (1U << 2)) != 0);   /* taillight */
}

static void test_common_pages(void)
{
    struct bike_light bl;
    uint8_t p[8];

    bike_light_init(&bl, 0xAABBCCDDU);
    for (int i = 0; i < 4; i++) {
        bike_light_next_page(&bl, p);
        if (p[0] == 0x50) {
            CHECK(p[4] == 0xFF && p[5] == 0x00); /* development manufacturer ID */
        }
        if (p[0] == 0x51) {
            CHECK(p[4] == 0xDD && p[5] == 0xCC && p[6] == 0xBB && p[7] == 0xAA);
        }
    }
}

static void test_command_modes(void)
{
    struct bike_light bl;
    uint8_t d[8];
    enum lamp_mode m = LAMP_OFF;

    bike_light_init(&bl, 1U);

    /* mode 6 (slow flash), principal light only */
    make_settings(d, 0, 0x00, 7, 0, 6);
    CHECK(bike_light_handle_rx(&bl, d, &m));
    CHECK(m == LAMP_FLASH);
    CHECK(bl.cmd_seq == 7);

    /* mode 9 (auto) to all lights, any type */
    make_settings(d, 0, 0xF8, 8, 0, 9);
    CHECK(bike_light_handle_rx(&bl, d, &m));
    CHECK(m == LAMP_SMART);

    /* all lights, headlight only: not for a taillight */
    make_settings(d, 0, 0x08, 9, 0, 5);
    CHECK(!bike_light_handle_rx(&bl, d, &m));
    CHECK(bl.cmd_seq == 9); /* sequence is still acknowledged */

    /* all lights, taillight type */
    make_settings(d, 0, 0x28, 10, 0, 5);
    CHECK(bike_light_handle_rx(&bl, d, &m));
    CHECK(m == LAMP_STEADY);

    /* unsupported mode (fast flash) is ignored */
    make_settings(d, 0, 0x00, 11, 0, 7);
    CHECK(!bike_light_handle_rx(&bl, d, &m));

    /* addressed to another light index */
    make_settings(d, 2, 0x00, 12, 0, 5);
    CHECK(!bike_light_handle_rx(&bl, d, &m));

    /* sub-light command: we have none */
    make_settings(d, 0, 0x01, 13, 0, 5);
    CHECK(!bike_light_handle_rx(&bl, d, &m));
}

static void test_command_beam_off_and_resume(void)
{
    struct bike_light bl;
    uint8_t d[8];
    enum lamp_mode m = LAMP_STEADY;

    bike_light_init(&bl, 1U);
    bike_light_set_mode(&bl, LAMP_FLASH);

    make_settings(d, 0, 0x00, 1, 1, 0); /* beam = off */
    CHECK(bike_light_handle_rx(&bl, d, &m));
    CHECK(m == LAMP_OFF);
    bike_light_set_mode(&bl, m);

    make_settings(d, 0, 0x00, 2, 2, 0); /* beam low, no mode: resume */
    CHECK(bike_light_handle_rx(&bl, d, &m));
    CHECK(m == LAMP_FLASH);

    bike_light_set_mode(&bl, LAMP_FLASH);
    make_settings(d, 0, 0x00, 3, 2, 0); /* already on: nothing */
    CHECK(!bike_light_handle_rx(&bl, d, &m));
}

static void test_command_restarts_pattern(void)
{
    struct bike_light bl;
    uint8_t d[8];
    uint8_t p[8];
    enum lamp_mode m;

    bike_light_init(&bl, 1U);
    bike_light_next_page(&bl, p);
    bike_light_next_page(&bl, p);

    make_settings(d, 0, 0x00, 5, 0, 99); /* unsupported mode but a command */
    (void)bike_light_handle_rx(&bl, d, &m);
    bike_light_next_page(&bl, p);
    CHECK(p[0] == 0x01);
    CHECK(p[4] == 5);
}

static void test_request_page(void)
{
    struct bike_light bl;
    uint8_t d[8] = { BL_PAGE_REQUEST, 0xFF, 0xFF, 0xFF, 0xFF, 2, BL_PAGE_SUPPLEMENTARY, 1 };
    uint8_t p[8];
    enum lamp_mode m;

    bike_light_init(&bl, 1U);
    bike_light_set_battery(&bl, BL_BATT_OK, 55U);
    bl.event_repeats = 0;

    CHECK(!bike_light_handle_rx(&bl, d, &m));
    bike_light_next_page(&bl, p);
    CHECK(p[0] == 0x13 && p[7] == 55);
    bike_light_next_page(&bl, p);
    CHECK(p[0] == 0x13);
    bike_light_next_page(&bl, p);
    CHECK(p[0] != 0x13);

    /* unsupported page request is ignored, wrong command type too */
    d[6] = 0x05;
    (void)bike_light_handle_rx(&bl, d, &m);
    CHECK(bl.request_repeats == 0);
    d[6] = BL_PAGE_SUPPLEMENTARY;
    d[7] = 2;
    (void)bike_light_handle_rx(&bl, d, &m);
    CHECK(bl.request_repeats == 0);
}

static void test_unknown_pages(void)
{
    struct bike_light bl;
    uint8_t d[8] = { 0x99, 0, 0, 0, 0, 0, 0, 0 };
    enum lamp_mode m;

    bike_light_init(&bl, 1U);
    CHECK(!bike_light_handle_rx(&bl, d, &m));
    d[0] = BL_PAGE_CONNECT;
    CHECK(!bike_light_handle_rx(&bl, d, &m));
}

/* Messages captured from a Wahoo ELEMNT BOLT v2 (see VERIFY.md): it forms a
 * network with the single light before it sends light settings */
static void test_wahoo_connect_then_settings(void)
{
    struct bike_light bl;
    const uint8_t connect[8] = { 0x21, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t slow_flash[8] = { 0x22, 0x01, 0x20, 0x07, 0x00, 0x00, 0x1A, 0xFF };
    const uint8_t disconnect[8] = { 0x20, 0x01, 0x05, 0, 0, 0, 0, 0 };
    enum lamp_mode m = LAMP_OFF;
    uint8_t p[8];

    bike_light_init(&bl, 0x1234U);
    bike_light_set_mode(&bl, LAMP_STEADY);

    /* settings for light index 1 are ignored while unconnected */
    CHECK(!bike_light_handle_rx(&bl, slow_flash, &m));

    /* connect: no mode change, light index 1, controller ID echoed */
    CHECK(!bike_light_handle_rx(&bl, connect, &m));
    bike_light_next_page(&bl, p);
    CHECK(p[0] == 0x01 && p[1] == 1 && p[4] == 0x00);

    /* settings now apply: mode 6 (slow flash), beam low */
    CHECK(bike_light_handle_rx(&bl, slow_flash, &m));
    CHECK(m == LAMP_FLASH);
    bike_light_set_mode(&bl, m);
    bike_light_next_page(&bl, p);
    CHECK(p[0] == 0x01 && p[1] == 1 && p[4] == 0x07 && (p[6] >> 2) == 6);

    /* the connected rotation contains the channel ID page (18) */
    for (int i = 0; i < 12; i++) {
        bike_light_next_page(&bl, p);
        if (p[0] == 0x12) {
            break;
        }
    }
    CHECK(p[0] == 0x12);
    CHECK(p[5] == 0x34 && p[6] == 0x12 && p[7] == BL_TRANSMISSION_TYPE);
    CHECK(p[1] == 0xFF && p[4] == 0xFF);

    /* disconnect: back to light index 0, page 18 gone, controller ID echoed */
    CHECK(!bike_light_handle_rx(&bl, disconnect, &m));
    bike_light_next_page(&bl, p);
    CHECK(p[0] == 0x01 && p[1] == 0 && p[4] == 0x05);
    for (int i = 0; i < 12; i++) {
        bike_light_next_page(&bl, p);
        CHECK(p[0] != 0x12);
    }
    CHECK(!bike_light_handle_rx(&bl, slow_flash, &m));
}

static void test_connect_state_setting_and_secondary(void)
{
    struct bike_light bl;
    uint8_t connect[8] = { 0x21, 0xFF, 0x00, 0x09, 0x10, 0x00, 0x00, 0x00 };
    enum lamp_mode m = LAMP_OFF;

    bike_light_init(&bl, 1U);

    /* index 255 with light state "flashing" (bits 3:4 = 2): only a state change */
    CHECK(bike_light_handle_rx(&bl, connect, &m) && m == LAMP_FLASH);
    CHECK(bl.light_index == 0 && bl.cmd_seq == 9);

    /* index 3 = secondary light: not supported, ignored without a state change */
    connect[1] = 3;
    CHECK(!bike_light_handle_rx(&bl, connect, &m));
    CHECK(bl.light_index == 0);

    /* connect as main light with state "steady" */
    connect[1] = 1;
    connect[4] = 0x18;
    CHECK(bike_light_handle_rx(&bl, connect, &m) && m == LAMP_STEADY);
    CHECK(bl.light_index == 1);

    /* already connected: a connect for another light is not for us */
    connect[1] = 2;
    CHECK(!bike_light_handle_rx(&bl, connect, &m));
    CHECK(bl.light_index == 1);
}

/* ANT-11: the controller went quiet */
static void test_silence_returns_to_unconnected_without_touching_mode(void)
{
    struct bike_light bl;
    const uint8_t connect[8] = { 0x21, 0x01, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00 };
    enum lamp_mode m = LAMP_OFF;
    uint8_t p[8];

    bike_light_init(&bl, 1U);
    bike_light_set_mode(&bl, LAMP_FLASH);
    bike_light_handle_rx(&bl, connect, &m);
    CHECK(bike_light_is_connected(&bl));

    bike_light_disconnect(&bl);
    CHECK(!bike_light_is_connected(&bl));
    bike_light_next_page(&bl, p); /* page 1 restarts the pattern */
    CHECK(p[0] == 0x01 && p[1] == 0);
    CHECK((p[6] >> 2) == 6); /* still slow flash */
    CHECK(p[4] == 0x07);     /* the last controller ID stays reported */

    bike_light_disconnect(&bl); /* already unconnected: nothing to do */
    CHECK(!bike_light_is_connected(&bl));
}

int main(void)
{
    test_default_rotation();
    test_event_restarts_pattern();
    test_page1_content();
    test_capabilities();
    test_common_pages();
    test_command_modes();
    test_command_beam_off_and_resume();
    test_command_restarts_pattern();
    test_request_page();
    test_unknown_pages();
    test_wahoo_connect_then_settings();
    test_connect_state_setting_and_secondary();
    test_silence_returns_to_unconnected_without_touching_mode();

    if (failures == 0) {
        printf("all bike_light tests passed\n");
        return 0;
    }
    printf("%d failure(s)\n", failures);
    return 1;
}
