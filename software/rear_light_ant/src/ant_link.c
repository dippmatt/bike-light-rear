/*
 * ANT SoftDevice glue, see ant_link.h.
 */

#include "ant_link.h"

#include <stddef.h>

#include "app_events.h"
#include "config.h"
#ifdef HAYSTACK
#include "haystack.h"
#endif
#include "rear_link.h"
#include "sd_includes.h"
#include "soft_timer.h"
#include "stats.h"

#define ANT_CHANNEL 0U
#define ANT_NETWORK 0U
#define REAR_CHANNEL 1U /* front light only: slave channel to the rear (REAR-1) */

#define SD_EVT_IRQ_PRIORITY 6U

/* ANT+ network key: not in the repository. The developer provides it in
 * keys.local.mk (ANT_PLUS_NETWORK_KEY); the Makefile generates ant_keys.h.
 * Get it from Garmin as an ANT+ Adopter:
 * https://developer.garmin.com/ant-program/ant-ant-plus/network-keys/
 * It may only be used by devices that follow an ANT+ device profile. */
#include "ant_keys.h"

static const uint8_t ant_plus_network_key[8] = { ANT_PLUS_NETWORK_KEY_BYTES };

/* One channel (front light: two), no encryption, minimum burst queue
 * (bursts are not used), internal event queue */
#define ANT_STACK_RAM_SIZE ANT_ENABLE_GET_REQUIRED_SPACE(ANT_CHANNELS, 0, 64, 0)
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

#if REAR_CONTROL
static struct rear_link rear;
static enum ant_channel_state rear_state = ANT_CH_CLOSED;
static uint16_t rear_exclude;     /* device to put on the exclusion list once closed */
static uint8_t rear_excluded;     /* entries in the exclusion list (max 4) */
static uint32_t rear_retry_ticks;
static bool rear_reopen;          /* woken while the channel was still closing */
#endif

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

#if REAR_CONTROL
/* --- Rear control channel (front light, SPEC section 12) -------------------- */

/* Slave on the ANT+ network, frequency and period of the bike lights; low
 * priority search only, so the light channel keeps transmitting (REAR-1) */
static uint32_t configure_rear_channel(void)
{
    uint32_t err;

    err = sd_ant_channel_assign(REAR_CHANNEL, CHANNEL_TYPE_SLAVE, ANT_NETWORK, 0U);
    if (err == NRF_SUCCESS) {
        err = sd_ant_channel_radio_freq_set(REAR_CHANNEL, ANT_RF_FREQUENCY);
    }
    if (err == NRF_SUCCESS) {
        err = sd_ant_channel_period_set(REAR_CHANNEL, ANT_CHANNEL_PERIOD);
    }
    if (err == NRF_SUCCESS) {
        err = sd_ant_channel_rx_search_timeout_set(REAR_CHANNEL, 0U); /* no high-priority search */
    }
    if (err == NRF_SUCCESS) {
        err = sd_ant_channel_low_priority_rx_search_timeout_set(REAR_CHANNEL, REAR_SEARCH_TIMEOUT);
    }
    record_error(0x30U, err);
    return err;
}

/* Wildcard device number with proximity search for the first find; once a
 * rear is known, ANT keeps its ID and a reopen searches only for it (REAR-2) */
static void rear_open_now(void)
{
    uint32_t err;
    bool known = (rear.device_number != 0U && rear.phase == REAR_FOUND);

    err = sd_ant_channel_id_set(REAR_CHANNEL, known ? rear.device_number : 0U,
                                ANT_DEVICE_TYPE_BIKE_LIGHT, 0U);
    if (err == NRF_SUCCESS) {
        err = sd_ant_prox_search_set(REAR_CHANNEL,
                                     known ? PROXIMITY_THRESHOLD_OFF : PROXIMITY_THRESHOLD_CUSTOM,
                                     (uint8_t)(int8_t)REAR_PROXIMITY_DBM);
    }
    if (err == NRF_SUCCESS) {
        err = sd_ant_channel_open(REAR_CHANNEL);
    }
    record_error(0x31U, err);
    if (err == NRF_SUCCESS) {
        rear_state = ANT_CH_OPEN;
    }
    rear_retry_ticks = 0U;
    proto_stats.rear_phase = (uint8_t)rear.phase;
}

static void rear_close(void)
{
    if (rear_state == ANT_CH_OPEN && sd_ant_channel_close(REAR_CHANNEL) == NRF_SUCCESS) {
        rear_state = ANT_CH_CLOSING;
    }
}

static void rear_do(enum rear_link_action action, uint16_t device_number)
{
    uint8_t page[BL_PAGE_SIZE];
    uint32_t err;

    switch (action) {
    case REAR_ACTION_SEND:
        rear_link_build(&rear, page);
        err = sd_ant_acknowledge_message_tx(REAR_CHANNEL, BL_PAGE_SIZE, page);
        record_error(0x32U, err);
        if (err == NRF_SUCCESS) {
            proto_stats.rear_sent++;
            proto_stats.rear_last_cmd = page[6];
        } else {
            rear_link_on_closed(&rear); /* not queued: try again with the next page */
        }
        break;
    case REAR_ACTION_EXCLUDE:
        rear_exclude = device_number; /* added to the list when the channel has closed */
        rear_close();
        break;
    case REAR_ACTION_NONE:
    default:
        break;
    }
}

static void rear_on_closed(void)
{
    uint8_t id[ANT_ID_SIZE];

    rear_state = ANT_CH_CLOSED;
    rear_link_on_closed(&rear);

    if (rear_exclude != 0U) {
        id[0] = (uint8_t)(rear_exclude & 0xFFU);
        id[1] = (uint8_t)(rear_exclude >> 8);
        id[2] = ANT_DEVICE_TYPE_BIKE_LIGHT;
        id[3] = 0U;
        (void)sd_ant_id_list_add(REAR_CHANNEL, id, (uint8_t)(rear_excluded % 4U));
        if (rear_excluded < 4U) {
            rear_excluded++;
        }
        (void)sd_ant_id_list_config(REAR_CHANNEL, rear_excluded, 1U); /* exclusion list */
        proto_stats.rear_excluded++;
        rear_exclude = 0U;
        if (want_open) {
            rear_open_now(); /* search for the next candidate */
        }
    } else if (rear_reopen && want_open) {
        rear_open_now();
    }
    rear_reopen = false;
    /* A search timeout is retried from the battery tick (REAR-2) */
}

static void rear_handle_event(uint8_t event, const ANT_MESSAGE *msg)
{
    uint16_t number = 0U;
    uint8_t type;
    uint8_t trans;

    switch (event) {
    case EVENT_RX:
        if (msg->ANT_MESSAGE_ucMesgID != MESG_BROADCAST_DATA_ID &&
            msg->ANT_MESSAGE_ucMesgID != MESG_ACKNOWLEDGED_DATA_ID) {
            break;
        }
        /* Signal strength of the rear, for tuning REAR_PROXIMITY_DBM */
        if ((msg->ANT_MESSAGE_ucExtMesgBF & ANT_EXT_MESG_BITFIELD_RSSI) != 0U &&
            msg->ANT_MESSAGE_aucExtData[RSSI_TYPE_OFFSET] == RSSI_DBM_TYPE) {
            proto_stats.rear_rssi = (int8_t)msg->ANT_MESSAGE_aucExtData[RSSI_TYPE_DBM_VALUE];
            proto_stats.rear_rssi_count++;
        }
        (void)sd_ant_channel_id_get(REAR_CHANNEL, &number, &type, &trans);
        rear_do(rear_link_on_page(&rear, number, msg->ANT_MESSAGE_aucPayload), number);
        proto_stats.rear_device = (rear.phase == REAR_FOUND) ? rear.device_number : 0U;
        proto_stats.rear_phase = (uint8_t)rear.phase;
        break;
    case EVENT_TRANSFER_TX_COMPLETED:
        proto_stats.rear_acked++;
        rear_do(rear_link_on_tx_done(&rear, true), 0U);
        break;
    case EVENT_TRANSFER_TX_FAILED:
        proto_stats.rear_failed++;
        rear_do(rear_link_on_tx_done(&rear, false), 0U);
        break;
    case EVENT_CHANNEL_CLOSED:
        rear_on_closed();
        break;
    default:
        break;
    }
}
#endif

uint32_t ant_link_init(ant_mode_request_cb on_mode_request)
{
    const nrf_clock_lf_cfg_t clock_cfg = LFCLK_CONFIG;
    ANT_ENABLE ant_enable = {
        .ucTotalNumberOfChannels = ANT_CHANNELS,
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

#ifdef HAYSTACK
    /* Not fatal: the light works without the Find My advert (ADR-0011) */
    record_error(0x34U, haystack_init());
#endif

#if REAR_CONTROL
    rear_link_init(&rear);
    (void)configure_rear_channel(); /* not fatal: the light works without rear control */
    /* Received messages carry their RSSI (diagnostics only, REAR-2) */
    record_error(0x33U, sd_ant_lib_config_set(ANT_LIB_CONFIG_MESG_OUT_INC_RSSI));
#endif

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

#if REAR_CONTROL
    /* Each wake looks for the nearest rear anew (REAR-2) */
    rear_link_forget(&rear);
    rear_excluded = 0U;
    (void)sd_ant_id_list_config(REAR_CHANNEL, 0U, 1U);
    proto_stats.rear_device = 0U;
    if (rear_state == ANT_CH_CLOSED) {
        rear_open_now();
    } else if (rear_state == ANT_CH_CLOSING) {
        rear_reopen = true; /* opened by the close event */
    }
#endif
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

#if REAR_CONTROL
    rear_close();
#endif
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

    /* SoC events: only the radio timeslot ones matter (Find My advert), but
     * the queue must not fill up */
#ifdef HAYSTACK
    haystack_poll();
#endif
    while (sd_evt_get(&soc_event) == NRF_SUCCESS) {
#ifdef HAYSTACK
        haystack_on_soc_event(soc_event);
#endif
    }

    while (sd_ant_event_get(&channel, &event, msg.ANT_MESSAGE_aucMessage) == NRF_SUCCESS) {
#if REAR_CONTROL
        if (channel == REAR_CHANNEL) {
            rear_handle_event(event, &msg);
            continue;
        }
#endif
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

#if REAR_CONTROL
    /* After a search timeout, search again every few ticks (REAR-2) */
    if (rear_state == ANT_CH_CLOSED && ++rear_retry_ticks >= REAR_RETRY_BATTERY_TICKS) {
        rear_open_now();
    }
#endif

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

void ant_link_rear_request(enum system_state state)
{
#if REAR_CONTROL
    if (available) {
        rear_do(rear_link_request(&rear, state), 0U);
    }
#else
    (void)state;
#endif
}

void ant_link_rear_flush(uint32_t timeout_ms)
{
#if REAR_CONTROL
    uint32_t start = soft_timer_uptime_ms();

    if (!available) {
        return;
    }
    /* Deliver the Off of a power-off before the channels close (REAR-6). The
     * main loop is not running here, so the SoftDevice events are pumped. */
    while (rear_link_busy(&rear) && rear_state == ANT_CH_OPEN &&
           (uint32_t)(soft_timer_uptime_ms() - start) < timeout_ms) {
        ant_link_process();
    }
    rear_link_drop(&rear);
#else
    (void)timeout_ms;
#endif
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
