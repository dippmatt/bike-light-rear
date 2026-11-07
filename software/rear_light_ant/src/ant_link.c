/*
 * ANT SoftDevice glue, see ant_link.h.
 */

#include "ant_link.h"

#include <stddef.h>

#include "app_events.h"
#include "config.h"
#include "sd_includes.h"
#include "stats.h"

#define ANT_CHANNEL 0U
#define ANT_NETWORK 0U

#define SD_EVT_IRQ_PRIORITY 6U

/* ANT+ network key: not in the repository. The developer provides it in
 * keys.local.mk (ANT_PLUS_NETWORK_KEY); the Makefile generates ant_keys.h.
 * Get it from Garmin as an ANT+ Adopter:
 * https://developer.garmin.com/ant-program/ant-ant-plus/network-keys/
 * It may only be used by devices that follow an ANT+ device profile. */
#include "ant_keys.h"

static const uint8_t ant_plus_network_key[8] = { ANT_PLUS_NETWORK_KEY_BYTES };

/* One channel, no encryption, minimum burst queue (bursts are not used),
 * internal event queue */
#define ANT_STACK_RAM_SIZE ANT_ENABLE_GET_REQUIRED_SPACE(1, 0, 64, 0)
static uint8_t ant_stack_ram[ANT_STACK_RAM_SIZE] __attribute__((aligned(4)));

#if defined(LFCLK_XTAL)
#define LFCLK_CONFIG { NRF_CLOCK_LF_SRC_XTAL, 0, 0, NRF_CLOCK_LF_ACCURACY_20_PPM }
#elif defined(LFCLK_SYNTH)
/* Derived from the 32 MHz crystal: accurate, but the high-frequency crystal
 * stays on, which costs current. Not a tested ANT configuration either. */
#define LFCLK_CONFIG { NRF_CLOCK_LF_SRC_SYNTH, 0, 0, NRF_CLOCK_LF_ACCURACY_250_PPM }
#elif defined(LFCLK_RC)
/* Internal RC oscillator: calibrated every 4 s (rc_ctiv 16 in 1/4 s units,
 * as recommended by the SoftDevice documentation). Not a configuration that
 * the vendor tested with ANT, accepted in ADR-0006. */
#define LFCLK_CONFIG { NRF_CLOCK_LF_SRC_RC, 16, 2, NRF_CLOCK_LF_ACCURACY_500_PPM }
#else
#error "LFCLK must be rc, synth or xtal"
#endif

nrf_nvic_state_t nrf_nvic_state = { { 0U, 0U }, 0U };

volatile struct proto_stats proto_stats = { .magic = STATS_MAGIC };

static struct bike_light light;
static ant_mode_request_cb mode_request_cb;
static bool available;
static enum ant_channel_state channel_state = ANT_CH_CLOSED;
static bool want_open;
static uint32_t silent_ticks;

static void record_error(uint32_t site, uint32_t err)
{
    if (err != NRF_SUCCESS) {
        proto_stats.last_error = err;
        proto_stats.last_error_site = site;
    }
}

/* A SoftDevice fault (for example a memory access violation) is not
 * recoverable: keep the details for `make stats` and reset (ANT-13). */
static void sd_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    proto_stats.sd_fault_id = id;
    proto_stats.sd_fault_pc = pc;
    proto_stats.sd_fault_info = info;
    NVIC_SystemReset();
}

void SD_EVT_IRQHandler(void)
{
    (void)app_event_post(EVT_SD, 0U);
}

static void queue_next_page(void)
{
    uint8_t page[BL_PAGE_SIZE];
    uint32_t err;

    bike_light_next_page(&light, page);
    proto_stats.light_index = light.light_index;
    err = sd_ant_broadcast_message_tx(ANT_CHANNEL, BL_PAGE_SIZE, page);
    record_error(0x10U, err);
    if (err == NRF_SUCCESS) {
        for (unsigned int i = 0; i < BL_PAGE_SIZE; i++) {
            proto_stats.last_tx[i] = page[i];
        }
    }
}

static uint32_t configure_channel(void)
{
    uint32_t err;

    err = sd_ant_network_address_set(ANT_NETWORK, ant_plus_network_key);
    if (err != NRF_SUCCESS) {
        record_error(0x01U, err);
        return err;
    }
    /* Bidirectional master: the controller sends commands as acknowledged data */
    err = sd_ant_channel_assign(ANT_CHANNEL, CHANNEL_TYPE_MASTER, ANT_NETWORK, 0U);
    if (err != NRF_SUCCESS) {
        record_error(0x02U, err);
        return err;
    }
    err = sd_ant_channel_id_set(ANT_CHANNEL, bike_light_device_number(NRF_FICR->DEVICEID[0]),
                                ANT_DEVICE_TYPE_BIKE_LIGHT, BL_TRANSMISSION_TYPE);
    if (err != NRF_SUCCESS) {
        record_error(0x03U, err);
        return err;
    }
    err = sd_ant_channel_radio_freq_set(ANT_CHANNEL, ANT_RF_FREQUENCY);
    if (err != NRF_SUCCESS) {
        record_error(0x04U, err);
        return err;
    }
    err = sd_ant_channel_period_set(ANT_CHANNEL, ANT_CHANNEL_PERIOD);
    if (err != NRF_SUCCESS) {
        record_error(0x05U, err);
    }
    return err;
}

uint32_t ant_link_init(ant_mode_request_cb on_mode_request)
{
    const nrf_clock_lf_cfg_t clock_cfg = LFCLK_CONFIG;
    ANT_ENABLE ant_enable = {
        .ucTotalNumberOfChannels = 1U,
        .ucNumberOfEncryptedChannels = 0U,
        .usNumberOfEvents = 0U,
        .pucMemoryBlockStartLocation = ant_stack_ram,
        .usMemoryBlockByteSize = ANT_STACK_RAM_SIZE,
    };
    uint32_t err;

    mode_request_cb = on_mode_request;
    bike_light_init(&light, NRF_FICR->DEVICEID[0]);

#ifdef FORCE_SD_FAILURE
    err = NRF_ERROR_INTERNAL; /* test build: exercise the "no SoftDevice" path (SYS-5) */
    (void)clock_cfg;
    (void)sd_fault_handler;
#else
    err = sd_softdevice_enable(&clock_cfg, sd_fault_handler, ANT_LICENSE_KEY);
#endif
    if (err != NRF_SUCCESS) {
        record_error(0x20U, err);
        proto_stats.ant_init_error = err;
        return err;
    }

    (void)sd_nvic_SetPriority(SD_EVT_IRQn, SD_EVT_IRQ_PRIORITY);
    (void)sd_nvic_EnableIRQ(SD_EVT_IRQn);

    err = sd_ant_enable(&ant_enable);
    if (err != NRF_SUCCESS) {
        record_error(0x21U, err);
        proto_stats.ant_init_error = err;
        return err;
    }

    err = configure_channel();
    if (err != NRF_SUCCESS) {
        proto_stats.ant_init_error = err;
        return err;
    }

    available = true;
    return 0U;
}

bool ant_link_available(void)
{
    return available;
}

static void open_channel_now(void)
{
    uint32_t err;

    /* The first page has to be queued before the channel opens */
    queue_next_page();
    err = sd_ant_channel_open(ANT_CHANNEL);
    record_error(0x06U, err);
    if (err == NRF_SUCCESS) {
        channel_state = ANT_CH_OPEN;
    }
    proto_stats.channel_state = (uint8_t)channel_state;
}

void ant_link_open(void)
{
    if (!available) {
        return;
    }

    want_open = true;
    silent_ticks = 0U;
    if (channel_state == ANT_CH_CLOSED) {
        open_channel_now();
    }
    /* ANT_CH_CLOSING: opened by the close event, ANT_CH_OPEN: nothing to do */
}

void ant_link_close(void)
{
    uint32_t err;

    if (!available) {
        return;
    }

    want_open = false;
    if (channel_state != ANT_CH_OPEN) {
        return;
    }
    err = sd_ant_channel_close(ANT_CHANNEL);
    record_error(0x07U, err);
    if (err == NRF_SUCCESS) {
        channel_state = ANT_CH_CLOSING; /* EVENT_CHANNEL_CLOSED follows */
    }
    proto_stats.channel_state = (uint8_t)channel_state;
}

static void count_event(uint8_t event)
{
    unsigned int index = (event < 0x40U) ? event : (event == EVENT_RX ? 0x40U : 0x41U);

    if (proto_stats.event_count[index] != UINT16_MAX) {
        proto_stats.event_count[index]++;
    }
}

static void handle_received(const ANT_MESSAGE *msg)
{
    const uint8_t *payload = msg->ANT_MESSAGE_aucPayload;
    volatile struct rx_log_entry *entry;
    enum lamp_mode requested;

    if (msg->ANT_MESSAGE_ucMesgID != MESG_BROADCAST_DATA_ID &&
        msg->ANT_MESSAGE_ucMesgID != MESG_ACKNOWLEDGED_DATA_ID) {
        proto_stats.rx_other++;
        proto_stats.rx_other_id = msg->ANT_MESSAGE_ucMesgID;
        return; /* bursts and other message types are not decoded */
    }

    silent_ticks = 0U; /* any controller message is a sign of life (ANT-11) */

    entry = &proto_stats.rx_log[proto_stats.rx_log_next % STATS_RX_LOG_LEN];
    proto_stats.rx_messages++;
    entry->seq = proto_stats.rx_log_next++;
    entry->msg_id = msg->ANT_MESSAGE_ucMesgID;
    for (unsigned int i = 0; i < BL_PAGE_SIZE; i++) {
        proto_stats.last_rx[i] = payload[i];
        entry->data[i] = payload[i];
    }

    if (payload[0] == BL_PAGE_SETTINGS) {
        proto_stats.rx_commands++;
    } else if (payload[0] == BL_PAGE_REQUEST) {
        proto_stats.rx_requests++;
    }

    if (bike_light_handle_rx(&light, payload, &requested) && mode_request_cb != NULL) {
        mode_request_cb(requested);
    }
}

void ant_link_process(void)
{
    uint8_t channel;
    uint8_t event;
    ANT_MESSAGE msg;
    uint32_t soc_event;

    if (!available) {
        return;
    }

    /* SoC events are not used, but the queue must not fill up */
    while (sd_evt_get(&soc_event) == NRF_SUCCESS) {
    }

    while (sd_ant_event_get(&channel, &event, msg.ANT_MESSAGE_aucMessage) == NRF_SUCCESS) {
        count_event(event);
        switch (event) {
        case EVENT_TX:
            proto_stats.tx_events++;
            if (channel_state == ANT_CH_OPEN) {
                queue_next_page();
            }
            break;
        case EVENT_RX:
            handle_received(&msg);
            break;
        case EVENT_CHANNEL_CLOSED:
            channel_state = ANT_CH_CLOSED;
            proto_stats.channel_state = (uint8_t)channel_state;
            if (want_open) {
                open_channel_now(); /* awake again before the close completed */
            }
            break;
        default:
            break;
        }
    }
}

void ant_link_report_mode(enum lamp_mode mode)
{
    bike_light_set_mode(&light, mode);
}

void ant_link_report_battery(uint8_t status, uint8_t percent)
{
    bike_light_set_battery(&light, status, percent);
}

void ant_link_on_battery_tick(void)
{
    if (!available || !want_open) {
        return;
    }

    if (channel_state == ANT_CH_CLOSED) {
        open_channel_now(); /* a failed open is retried (ANT-1) */
    }

    if (bike_light_is_connected(&light)) {
        silent_ticks++;
        if (silent_ticks >= ANT_SILENCE_TIMEOUT_BATTERY_TICKS) {
            bike_light_disconnect(&light); /* the beam is not touched (ANT-11) */
            silent_ticks = 0U;
            proto_stats.silence_timeouts++;
        }
    } else {
        silent_ticks = 0U;
    }
}

void ant_link_sleep_until_event(void)
{
    uint8_t nested;

    /* With interrupts masked a pending interrupt still ends the wait (SEVONPEND,
     * set in main), so an event posted between the check and the sleep is not lost */
    (void)sd_nvic_critical_region_enter(&nested);
    if (!app_event_pending()) {
        if (available) {
            (void)sd_app_evt_wait();
        } else {
            /* No SoftDevice: SEVONPEND turns a pending interrupt into an
             * event, the first WFE clears a stale one */
            __SEV();
            __WFE();
            __WFE();
        }
    }
    (void)sd_nvic_critical_region_exit(nested);
}
