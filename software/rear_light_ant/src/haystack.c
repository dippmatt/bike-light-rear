/*
 * Find My advertiser, see haystack.h.
 *
 * One timeslot carries one advert event: the same packet on channel 37, 38
 * and 39, each started from the DISABLED event of the one before. The
 * signal callback runs in the SoftDevice's interrupt context, so it only
 * touches the RADIO and TIMER0 registers and no SoftDevice call.
 *
 * There are two packets, one per key (main and backup chain), sent in turn.
 * AIRTAG_ROLLING (ADR-0012): the keys follow a cloned AirTag's schedule,
 * derived here from the provisioned key store. The main loop builds new
 * packets into the spare half of pdu[] and then flips active_pdu; the
 * callback only ever reads pdu[active_pdu]. Without AIRTAG_ROLLING both
 * packets carry the one fixed key.
 */

#include "haystack.h"

#include "config.h"
#include "haystack_adv.h"
#include "sd_includes.h"

#ifdef AIRTAG_ROLLING
#include "findmy_keys.h"
#include "key_store.h"
#include "soft_timer.h"
#else
#include "haystack_key.h"
#endif

#define PDU_HEADER_LEN 2U
#define PDU_LEN        (PDU_HEADER_LEN + HAYSTACK_ADDR_LEN + HAYSTACK_PAYLOAD_LEN)
#define PDU_SLOT       ((PDU_LEN + 3U) & ~3U) /* every packet word-aligned */

#define KEY_MAIN   0U /* primary chain, 15-minute keys */
#define KEY_BACKUP 1U /* secondary chain, daily keys */

#define ADV_CHANNELS    3U
#define BLE_ACCESS_ADDR 0x8E89BED6UL
#define BLE_CRC_INIT    0x555555UL
#define BLE_CRC_POLY    0x00065BUL

/* 3 x (ramp-up ~140 us + 47 bytes at 1 Mbit/s = 376 us) is about 1.6 ms */
#define SLOT_US         2400UL
#define SLOT_GUARD_US   300UL /* safety timer fires this long before the slot ends */
#define EARLIEST_TIMEOUT_US 200000UL

volatile struct haystack_stats haystack_stats;

#ifndef AIRTAG_ROLLING
static const uint8_t adv_key[HAYSTACK_KEY_LEN] = { HAYSTACK_KEY_BYTES };
#endif

/* RADIO FREQUENCY register (MHz above 2400) and DATAWHITEIV of the channels */
static const uint8_t channel_freq[ADV_CHANNELS] = { 2U, 26U, 80U };
static const uint8_t channel_index[ADV_CHANNELS] = { 37U, 38U, 39U };

static uint8_t pdu[2][2][PDU_SLOT] __attribute__((aligned(4))); /* [half][KEY_*] */
static volatile uint32_t active_pdu; /* written by the main loop, read in the callback */
static uint32_t next_key;           /* KEY_* sent in the next timeslot (callback only) */
static uint32_t channel;
static uint32_t radio_was_powered; /* RADIO POWER at the slot start, restored at its end */
#ifdef AIRTAG_ROLLING
static struct key_store_pos pos;   /* keys in use */
static uint8_t backup_key[FINDMY_ADV_KEY_LEN]; /* derived once a day, kept for each rebuild */
static uint32_t rotated_at_ms;     /* uptime of the last rotation (or boot) */
static bool rolling;               /* key store valid, rotation running */
#endif

static nrf_radio_request_t earliest_request; /* used from the main loop */
static nrf_radio_request_t next_request;     /* used from the signal callback */
static nrf_radio_signal_callback_return_param_t callback_return;

static void select_channel(uint32_t i)
{
    NRF_RADIO->FREQUENCY = channel_freq[i];
    NRF_RADIO->DATAWHITEIV = channel_index[i];
}

/* Header, address and payload of the advert for one 28-byte key */
static void build_pdu(uint8_t *buf, const uint8_t key[HAYSTACK_KEY_LEN])
{
    /* Header: PDU type ADV_NONCONN_IND (2), TxAdd = 1 (random address); then
     * the length of address plus data */
    buf[0] = 0x42U;
    buf[1] = (uint8_t)(HAYSTACK_ADDR_LEN + HAYSTACK_PAYLOAD_LEN);
    haystack_adv_build(key, &buf[PDU_HEADER_LEN], &buf[PDU_HEADER_LEN + HAYSTACK_ADDR_LEN]);
}

static void radio_start(void)
{
    /* The ANT stack leaves the RADIO powered down between its events and the
     * SoftDevice does not power it up for a timeslot: without this every
     * register write below is lost (found on the bench). Power-cycling also
     * resets the registers the ANT stack configured. */
    radio_was_powered = NRF_RADIO->POWER;
    NRF_RADIO->POWER = 0UL;
    NRF_RADIO->POWER = 1UL;
    NRF_RADIO->TXPOWER = HAYSTACK_TX_POWER << RADIO_TXPOWER_TXPOWER_Pos;
    NRF_RADIO->MODE = RADIO_MODE_MODE_Ble_1Mbit << RADIO_MODE_MODE_Pos;
    NRF_RADIO->PCNF0 = (8UL << RADIO_PCNF0_LFLEN_Pos) | (1UL << RADIO_PCNF0_S0LEN_Pos) |
                       (0UL << RADIO_PCNF0_S1LEN_Pos) |
                       (RADIO_PCNF0_PLEN_8bit << RADIO_PCNF0_PLEN_Pos);
    NRF_RADIO->PCNF1 = (37UL << RADIO_PCNF1_MAXLEN_Pos) | (0UL << RADIO_PCNF1_STATLEN_Pos) |
                       (3UL << RADIO_PCNF1_BALEN_Pos) |
                       (RADIO_PCNF1_ENDIAN_Little << RADIO_PCNF1_ENDIAN_Pos) |
                       (RADIO_PCNF1_WHITEEN_Enabled << RADIO_PCNF1_WHITEEN_Pos);
    NRF_RADIO->BASE0 = (BLE_ACCESS_ADDR << 8) & 0xFFFFFF00UL;
    NRF_RADIO->PREFIX0 = (BLE_ACCESS_ADDR >> 24) & RADIO_PREFIX0_AP0_Msk;
    NRF_RADIO->TXADDRESS = 0UL;
    NRF_RADIO->RXADDRESSES = 1UL;
    NRF_RADIO->CRCCNF = (RADIO_CRCCNF_LEN_Three << RADIO_CRCCNF_LEN_Pos) |
                        (RADIO_CRCCNF_SKIPADDR_Skip << RADIO_CRCCNF_SKIPADDR_Pos);
    NRF_RADIO->CRCINIT = BLE_CRC_INIT;
    NRF_RADIO->CRCPOLY = BLE_CRC_POLY;
    NRF_RADIO->PACKETPTR = (uint32_t)pdu[active_pdu][next_key];
    next_key ^= 1U; /* main, backup, main, ... (ADR-0012) */
    NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk | RADIO_SHORTS_END_DISABLE_Msk;

    channel = 0U;
    select_channel(0U);
    NRF_RADIO->EVENTS_DISABLED = 0UL;
    NRF_RADIO->INTENSET = RADIO_INTENSET_DISABLED_Msk;
    /* Found on the bench: while the ANT stack is active the SoftDevice leaves
     * the RADIO interrupt disabled in the NVIC during a timeslot (TIMER0's is
     * enabled), so the DISABLED interrupt would never arrive. We run at the
     * SoftDevice's own interrupt level here, so the direct write is allowed. */
    NVIC_EnableIRQ(RADIO_IRQn);
    NRF_RADIO->TASKS_TXEN = 1UL;

    /* Safety net: if the packets are not done shortly before the slot ends,
     * TIMER0 (1 MHz, started by the SoftDevice at the slot start) fires */
    NRF_TIMER0->EVENTS_COMPARE[0] = 0UL;
    NRF_TIMER0->CC[0] = SLOT_US - SLOT_GUARD_US;
    NRF_TIMER0->INTENSET = TIMER_INTENSET_COMPARE0_Msk;
}

static void radio_stop(void)
{
    NRF_TIMER0->INTENCLR = TIMER_INTENCLR_COMPARE0_Msk;
    NRF_TIMER0->EVENTS_COMPARE[0] = 0UL;
    NRF_RADIO->SHORTS = 0UL;
    NRF_RADIO->INTENCLR = 0xFFFFFFFFUL;
    NRF_RADIO->EVENTS_DISABLED = 0UL;
    NRF_RADIO->POWER = radio_was_powered;
}

/* Ends the slot and asks for the next one HAYSTACK_INTERVAL_MS after this one started */
static nrf_radio_signal_callback_return_param_t *end_and_request_next(void)
{
    radio_stop();
    next_request.request_type = NRF_RADIO_REQ_TYPE_NORMAL;
    next_request.params.normal.hfclk = NRF_RADIO_HFCLK_CFG_XTAL_GUARANTEED;
    next_request.params.normal.priority = NRF_RADIO_PRIORITY_NORMAL;
    next_request.params.normal.distance_us = HAYSTACK_INTERVAL_MS * 1000UL;
    next_request.params.normal.length_us = SLOT_US;
    callback_return.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_REQUEST_AND_END;
    callback_return.params.request.p_next = &next_request;
    return &callback_return;
}

static nrf_radio_signal_callback_return_param_t *signal_callback(uint8_t signal)
{
    callback_return.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE;

    switch (signal) {
    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_START:
        haystack_stats.starts++;
        radio_start();
        break;

    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_RADIO:
        if (NRF_RADIO->EVENTS_DISABLED != 0UL) {
            NRF_RADIO->EVENTS_DISABLED = 0UL;
            channel++;
            if (channel < ADV_CHANNELS) {
                select_channel(channel);
                NRF_RADIO->TASKS_TXEN = 1UL;
            } else {
                haystack_stats.adverts++;
                return end_and_request_next();
            }
        }
        break;

    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_TIMER0:
        if (NRF_TIMER0->EVENTS_COMPARE[0] != 0UL) {
            uint32_t guard = 1000UL;

            /* Stop whatever the radio is doing and wait for it to be idle */
            NRF_RADIO->SHORTS = 0UL;
            NRF_RADIO->EVENTS_DISABLED = 0UL;
            NRF_RADIO->TASKS_DISABLE = 1UL;
            while (NRF_RADIO->EVENTS_DISABLED == 0UL && guard-- > 0UL) {
            }
            haystack_stats.overruns++;
            return end_and_request_next();
        }
        break;

    default:
        break;
    }
    return &callback_return;
}

static uint32_t request_earliest(void)
{
    uint32_t err;

    earliest_request.request_type = NRF_RADIO_REQ_TYPE_EARLIEST;
    earliest_request.params.earliest.hfclk = NRF_RADIO_HFCLK_CFG_XTAL_GUARANTEED;
    earliest_request.params.earliest.priority = NRF_RADIO_PRIORITY_NORMAL;
    earliest_request.params.earliest.length_us = SLOT_US;
    earliest_request.params.earliest.timeout_us = EARLIEST_TIMEOUT_US;
    err = sd_radio_request(&earliest_request);
    if (err != NRF_SUCCESS) {
        haystack_stats.last_error = err;
    }
    return err;
}

#ifdef AIRTAG_ROLLING
/* Steps the backup chain up to the day of pos.index. True if it moved. */
static bool advance_backup(void)
{
    uint32_t want = findmy_secondary_index(pos.index);
    bool moved = false;

    while (pos.sks_index < want) {
        findmy_next_sk(pos.sks, pos.sks);
        pos.sks_index++;
        moved = true;
    }
    return moved;
}

/* Builds both adverts of pos into the spare packet half and makes it the
 * active one; derives the backup key only if it changed. False if the key
 * store holds an invalid master key. */
static bool load_keys(bool new_backup)
{
    uint8_t key[FINDMY_ADV_KEY_LEN];
    uint32_t spare = 1U - active_pdu;
    uint32_t t0 = soft_timer_uptime_ms();

    if (!findmy_adv_key(pos.master, pos.sk, key)) {
        return false;
    }
    if (new_backup && !findmy_adv_key(pos.master, pos.sks, backup_key)) {
        return false;
    }
    haystack_stats.derive_ms = soft_timer_uptime_ms() - t0;
    build_pdu(pdu[spare][KEY_MAIN], key);
    build_pdu(pdu[spare][KEY_BACKUP], backup_key);
    active_pdu = spare; /* one aligned word: the callback sees old or new, never half */
    haystack_stats.index = pos.index;
    haystack_stats.backup_index = pos.sks_index;
    return true;
}

/* Key store flash access through the SoftDevice, which owns the NVMC */
uint32_t key_store_flash_write(const uint32_t *dst, const uint32_t *src, uint32_t words)
{
    return sd_flash_write((uint32_t *)(uintptr_t)dst, src, words);
}

uint32_t key_store_flash_erase(const uint32_t *page)
{
    return sd_flash_page_erase((uint32_t)(uintptr_t)page / NRF_FICR->CODEPAGESIZE);
}
#endif

void haystack_poll(void)
{
#ifdef AIRTAG_ROLLING
    uint32_t steps;

    if (!rolling) {
        return;
    }
    key_store_poll();

    /* Never ahead of Apple's schedule, only behind (ADR-0012): rotate after
     * HAYSTACK_ROTATE_MS of uptime, a bit longer than 15 min, and catch up
     * every whole interval missed. */
    steps = (soft_timer_uptime_ms() - rotated_at_ms) / HAYSTACK_ROTATE_MS;
    if (steps == 0U) {
        return;
    }
    rotated_at_ms += steps * HAYSTACK_ROTATE_MS;
    while (steps-- > 0U) {
        findmy_next_sk(pos.sk, pos.sk); /* the old SK is gone from RAM */
        pos.index++;
    }
    key_store_save(pos.index, pos.sk);
    if (!load_keys(advance_backup())) {
        haystack_stats.last_error = NRF_ERROR_INVALID_DATA;
    }
#endif
}

uint32_t haystack_init(void)
{
    uint32_t err;

    active_pdu = 0U;
#ifdef AIRTAG_ROLLING
    if (!key_store_init((const uint32_t *)KEY_STORE_ADDR, &pos)) {
        haystack_stats.last_error = NRF_ERROR_NOT_FOUND; /* not provisioned */
        return NRF_ERROR_NOT_FOUND;
    }
    (void)advance_backup(); /* from the provisioning day to today's */
    if (!load_keys(true)) {
        haystack_stats.last_error = NRF_ERROR_INVALID_DATA;
        return NRF_ERROR_INVALID_DATA;
    }
    rotated_at_ms = 0U; /* uptime starts with soft_timer_init(), right after this */
    rolling = true;
#else
    build_pdu(pdu[0][KEY_MAIN], adv_key);
    build_pdu(pdu[0][KEY_BACKUP], adv_key);
#endif

    err = sd_radio_session_open(signal_callback);
    if (err != NRF_SUCCESS) {
        haystack_stats.last_error = err;
        return err;
    }
    return request_earliest();
}

void haystack_on_soc_event(uint32_t evt)
{
    switch (evt) {
    case NRF_EVT_RADIO_BLOCKED:
        haystack_stats.blocked++;
        (void)request_earliest();
        break;
    case NRF_EVT_RADIO_CANCELED:
        haystack_stats.canceled++;
        (void)request_earliest();
        break;
    case NRF_EVT_RADIO_SIGNAL_CALLBACK_INVALID_RETURN:
        haystack_stats.invalid++;
        (void)request_earliest();
        break;
    case NRF_EVT_RADIO_SESSION_IDLE:
        (void)request_earliest();
        break;
#ifdef AIRTAG_ROLLING
    case NRF_EVT_FLASH_OPERATION_SUCCESS:
        key_store_on_flash_done(true);
        break;
    case NRF_EVT_FLASH_OPERATION_ERROR:
        haystack_stats.flash_errors++;
        key_store_on_flash_done(false);
        break;
#endif
    default:
        break;
    }
}
