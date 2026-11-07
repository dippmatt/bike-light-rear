/*
 * Host tests of the rear control logic of the front light (SPEC section 12).
 * Built against the rear variant's bike_light.c, so the commands can be fed
 * to the rear light's own command handler (round trip, REAR-4).
 * Build and run with: make test
 */

#include <stdio.h>
#include <string.h>

#include "../src/bike_light.h"
#include "../src/config.h"
#include "../src/rear_link.h"

static int failures;

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);           \
            failures++;                                                      \
        }                                                                    \
    } while (0)

/* Page 1 of a light of the given type */
static void page1(uint8_t p[BL_PAGE_SIZE], uint8_t light_type)
{
    memset(p, 0, BL_PAGE_SIZE);
    p[0] = BL_PAGE_LIGHT_STATE;
    p[2] = (uint8_t)(light_type << 2);
}

/* Page 1 of the rear reporting an ANT+ mode */
static void rear_page1(uint8_t p[BL_PAGE_SIZE], uint8_t ant_mode)
{
    page1(p, REAR_LIGHT_TYPE);
    p[6] = (uint8_t)(ant_mode << 2);
}

static void find_rear(struct rear_link *rl, uint16_t number)
{
    uint8_t p[BL_PAGE_SIZE];

    page1(p, REAR_LIGHT_TYPE);
    (void)rear_link_on_page(rl, number, p);
}

/* The command for a front state, as the rear light understands it */
static bool rear_understands(enum system_state front, bool rear_connected, enum lamp_mode *mode)
{
    struct rear_link rl;
    struct bike_light rear;
    uint8_t cmd[BL_PAGE_SIZE];

    rear_link_init(&rl);
    find_rear(&rl, 0x1234U);
    (void)rear_link_request(&rl, front);
    rear_link_build(&rl, cmd);

    bike_light_init(&rear, 1U);
    bike_light_set_mode(&rear, LAMP_STEADY);
    if (rear_connected) {
        rear.light_index = BL_LIGHT_INDEX_MAIN; /* a Wahoo has connected it */
    }
    return bike_light_handle_rx(&rear, cmd, mode);
}

static void test_REAR4_round_trip_with_the_rear_handler(void)
{
    enum lamp_mode m;

    for (int connected = 0; connected <= 1; connected++) {
        CHECK(rear_understands(LED_SMART, connected, &m) && m == LAMP_SMART);
        CHECK(rear_understands(LED_STEADY, connected, &m) && m == LAMP_STEADY);
        CHECK(rear_understands(LED_BRIGHT, connected, &m) && m == LAMP_FLASH);
        CHECK(rear_understands(LED_OFF, connected, &m) && m == LAMP_OFF);
        CHECK(rear_understands(IDLE_CHARGING, connected, &m) && m == LAMP_OFF);
    }
}

static void test_REAR4_command_is_for_taillights_only(void)
{
    struct rear_link rl;
    struct bike_light other_front;
    uint8_t cmd[BL_PAGE_SIZE];
    enum lamp_mode m;

    rear_link_init(&rl);
    find_rear(&rl, 1U);
    (void)rear_link_request(&rl, LED_SMART);
    rear_link_build(&rl, cmd);
    CHECK(cmd[0] == BL_PAGE_SETTINGS && cmd[1] == 0U && cmd[2] == 0x28U);

    /* a headlight receiving it (light type filter of page 34) ignores it;
     * this test binary is the rear variant, so emulate by changing the type */
    cmd[2] = (uint8_t)((0U << 4) | 0x08U);
    bike_light_init(&other_front, 1U);
    CHECK(!bike_light_handle_rx(&other_front, cmd, &m));
}

static void test_REAR2_found_only_after_a_taillight_page1(void)
{
    struct rear_link rl;
    uint8_t p[BL_PAGE_SIZE];

    rear_link_init(&rl);
    CHECK(rear_link_request(&rl, LED_SMART) == REAR_ACTION_NONE); /* nothing found yet */

    /* first page of the acquired device is not page 1: still checking */
    memset(p, 0, sizeof(p));
    p[0] = BL_PAGE_CAPABILITIES;
    CHECK(rear_link_on_page(&rl, 0x4242U, p) == REAR_ACTION_NONE);
    CHECK(rl.phase == REAR_CHECKING && rl.device_number == 0x4242U);

    /* page 1 of a taillight: found, and the waiting command goes out */
    page1(p, REAR_LIGHT_TYPE);
    CHECK(rear_link_on_page(&rl, 0x4242U, p) == REAR_ACTION_SEND);
    CHECK(rl.phase == REAR_FOUND);
}

static void test_REAR2_headlight_is_excluded(void)
{
    struct rear_link rl;
    uint8_t p[BL_PAGE_SIZE];

    rear_link_init(&rl);
    (void)rear_link_request(&rl, LED_SMART);
    page1(p, 0U); /* another front light */
    CHECK(rear_link_on_page(&rl, 0x7777U, p) == REAR_ACTION_EXCLUDE);
    CHECK(rl.phase == REAR_SEARCHING && rl.pending);
    rear_link_on_closed(&rl);

    /* the next device found is a taillight */
    page1(p, REAR_LIGHT_TYPE);
    CHECK(rear_link_on_page(&rl, 0x1111U, p) == REAR_ACTION_SEND);
    CHECK(rl.device_number == 0x1111U);
}

static void test_REAR5_newest_command_wins(void)
{
    struct rear_link rl;
    uint8_t cmd[BL_PAGE_SIZE];

    rear_link_init(&rl);
    find_rear(&rl, 1U);
    CHECK(rear_link_request(&rl, LED_SMART) == REAR_ACTION_SEND);
    rear_link_build(&rl, cmd);
    uint8_t first_seq = cmd[3];

    /* a newer change while the first is in flight waits for its end */
    CHECK(rear_link_request(&rl, LED_STEADY) == REAR_ACTION_NONE);
    CHECK(rear_link_on_tx_done(&rl, true) == REAR_ACTION_SEND);
    rear_link_build(&rl, cmd);
    CHECK((cmd[6] >> 2) == REAR_ANT_MODE_STEADY && cmd[3] != first_seq);
    CHECK(rear_link_on_tx_done(&rl, true) == REAR_ACTION_NONE);
    CHECK(rl.pending && !rear_link_busy(&rl)); /* acked, the mode is being checked */
    uint8_t p[BL_PAGE_SIZE];
    rear_page1(p, REAR_ANT_MODE_STEADY);
    CHECK(rear_link_on_page(&rl, 1U, p) == REAR_ACTION_NONE);
    CHECK(!rl.pending);
}

/* ANT-8 on the rear: a command right after its own button press is acked
 * but ignored; the front sees the old mode in page 1 and sends again */
static void test_REAR5_ignored_command_is_sent_again(void)
{
    struct rear_link rl;
    uint8_t cmd[BL_PAGE_SIZE];
    uint8_t p[BL_PAGE_SIZE];

    rear_link_init(&rl);
    find_rear(&rl, 1U);
    CHECK(rear_link_request(&rl, LED_BRIGHT) == REAR_ACTION_SEND);
    rear_link_build(&rl, cmd);
    CHECK(rear_link_on_tx_done(&rl, true) == REAR_ACTION_NONE);

    rear_page1(p, REAR_ANT_MODE_AUTO); /* still Smart */
    for (unsigned int i = 0; i + 1U < REAR_VERIFY_PAGES; i++) {
        CHECK(rear_link_on_page(&rl, 1U, p) == REAR_ACTION_NONE);
    }
    CHECK(rear_link_on_page(&rl, 1U, p) == REAR_ACTION_SEND); /* window is over */
    rear_link_build(&rl, cmd);
    CHECK((cmd[6] >> 2) == REAR_ANT_MODE_FLASH);
    CHECK(rear_link_on_tx_done(&rl, true) == REAR_ACTION_NONE);

    rear_page1(p, REAR_ANT_MODE_FLASH); /* applied this time */
    CHECK(rear_link_on_page(&rl, 1U, p) == REAR_ACTION_NONE);
    CHECK(!rl.pending);

    /* other pages do not count as a mismatch */
    CHECK(rear_link_request(&rl, LED_STEADY) == REAR_ACTION_SEND);
    rear_link_build(&rl, cmd);
    (void)rear_link_on_tx_done(&rl, true);
    memset(p, 0, sizeof(p));
    p[0] = BL_PAGE_MANUFACTURER;
    for (unsigned int i = 0; i < 3U * REAR_VERIFY_PAGES; i++) {
        CHECK(rear_link_on_page(&rl, 1U, p) == REAR_ACTION_NONE);
    }
    CHECK(rl.verifying && rl.mismatches == 0U);
}

static void test_REAR5_off_expects_mode_0_and_new_command_replaces_check(void)
{
    struct rear_link rl;
    uint8_t cmd[BL_PAGE_SIZE];
    uint8_t p[BL_PAGE_SIZE];

    rear_link_init(&rl);
    find_rear(&rl, 1U);
    (void)rear_link_request(&rl, LED_OFF);
    rear_link_build(&rl, cmd);
    (void)rear_link_on_tx_done(&rl, true);
    rear_page1(p, 0U); /* the rear reports Off as mode 0 */
    CHECK(rear_link_on_page(&rl, 1U, p) == REAR_ACTION_NONE && !rl.pending);

    /* a new local change while the previous one is being checked */
    (void)rear_link_request(&rl, LED_STEADY);
    rear_link_build(&rl, cmd);
    (void)rear_link_on_tx_done(&rl, true);
    CHECK(rear_link_request(&rl, LED_SMART) == REAR_ACTION_SEND);
    rear_link_build(&rl, cmd);
    CHECK((cmd[6] >> 2) == REAR_ANT_MODE_AUTO);

    /* failed checks use the retry budget too */
    rear_link_init(&rl);
    find_rear(&rl, 1U);
    (void)rear_link_request(&rl, LED_BRIGHT);
    rear_page1(p, REAR_ANT_MODE_AUTO);
    for (unsigned int attempt = 0; attempt <= REAR_CMD_RETRIES; attempt++) {
        rear_link_build(&rl, cmd);
        (void)rear_link_on_tx_done(&rl, true);
        for (unsigned int i = 0; i < REAR_VERIFY_PAGES; i++) {
            (void)rear_link_on_page(&rl, 1U, p);
        }
    }
    CHECK(!rl.pending);
}

static void test_REAR5_retry_then_drop(void)
{
    struct rear_link rl;
    uint8_t cmd[BL_PAGE_SIZE];

    rear_link_init(&rl);
    find_rear(&rl, 1U);
    CHECK(rear_link_request(&rl, LED_SMART) == REAR_ACTION_SEND);
    for (unsigned int i = 0; i < REAR_CMD_RETRIES; i++) {
        rear_link_build(&rl, cmd);
        CHECK(rear_link_on_tx_done(&rl, false) == REAR_ACTION_SEND);
    }
    rear_link_build(&rl, cmd);
    CHECK(rear_link_on_tx_done(&rl, false) == REAR_ACTION_NONE);
    CHECK(!rl.pending && !rear_link_busy(&rl));
}

static void test_REAR5_waits_while_lost_and_after_wake(void)
{
    struct rear_link rl;
    uint8_t cmd[BL_PAGE_SIZE];
    uint8_t p[BL_PAGE_SIZE];

    rear_link_init(&rl);
    find_rear(&rl, 1U);
    (void)rear_link_request(&rl, LED_SMART);
    rear_link_build(&rl, cmd);
    rear_link_on_closed(&rl); /* channel closed with the transfer in flight */
    CHECK(rl.pending && !rl.in_flight && rl.phase == REAR_FOUND);

    /* the same rear is heard again: the command goes out */
    page1(p, REAR_LIGHT_TYPE);
    CHECK(rear_link_on_page(&rl, 1U, p) == REAR_ACTION_SEND);

    /* wake: the rear is forgotten, the pending command is kept for the next */
    rear_link_forget(&rl);
    CHECK(rl.phase == REAR_SEARCHING && rl.pending && !rear_link_busy(&rl));
    rear_link_drop(&rl);
    CHECK(!rl.pending);
}

int main(void)
{
    test_REAR4_round_trip_with_the_rear_handler();
    test_REAR4_command_is_for_taillights_only();
    test_REAR2_found_only_after_a_taillight_page1();
    test_REAR2_headlight_is_excluded();
    test_REAR5_newest_command_wins();
    test_REAR5_ignored_command_is_sent_again();
    test_REAR5_off_expects_mode_0_and_new_command_replaces_check();
    test_REAR5_retry_then_drop();
    test_REAR5_waits_while_lost_and_after_wake();

    if (failures == 0) {
        printf("all rear_link tests passed\n");
        return 0;
    }
    printf("%d failure(s)\n", failures);
    return 1;
}
