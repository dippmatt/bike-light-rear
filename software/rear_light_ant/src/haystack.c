/*
 * Find My advertiser, see haystack.h.
 *
 * One timeslot carries one advert event: the same packet on channel 37, 38
 * and 39, each started from the DISABLED event of the one before. The
 * signal callback runs in the SoftDevice's interrupt context, so it only
 * touches the RADIO and TIMER0 registers and no SoftDevice call.
 */

#include "haystack.h"

#include "config.h"
#include "haystack_adv.h"
#include "sd_includes.h"

#ifdef AIRTAG_ROLLING
#include "airtag_table.h"
#include "soft_timer.h"
#else
#include "haystack_key.h"
#endif

#define PDU_HEADER_LEN 2U
#define PDU_LEN        (PDU_HEADER_LEN + HAYSTACK_ADDR_LEN + HAYSTACK_PAYLOAD_LEN)

#define ADV_CHANNELS    3U
#define BLE_ACCESS_ADDR 0x8E89BED6UL
#define BLE_CRC_INIT    0x555555UL
#define BLE_CRC_POLY    0x00065BUL

/* 3 x (ramp-up ~140 us + 47 bytes at 1 Mbit/s = 376 us) is about 1.6 ms */
#define SLOT_US         2400UL
#define SLOT_GUARD_US   300UL /* safety timer fires this long before the slot ends */
#define EARLIEST_TIMEOUT_US 200000UL

#ifdef AIRTAG_ROLLING
/* Apple's real rotation interval (ADR-0012); the light has no RTC, so this
 * counts from boot (soft_timer_uptime_ms()) instead of wall-clock time. */
#define AIRTAG_ROTATE_MS (15UL * 60UL * 1000UL)
#endif

volatile struct haystack_stats haystack_stats;

#ifndef AIRTAG_ROLLING
static const uint8_t adv_key[HAYSTACK_KEY_LEN] = { HAYSTACK_KEY_BYTES };
#endif

/* RADIO FREQUENCY register (MHz above 2400) and DATAWHITEIV of the channels */
static const uint8_t channel_freq[ADV_CHANNELS] = { 2U, 26U, 80U };
static const uint8_t channel_index[ADV_CHANNELS] = { 37U, 38U, 39U };

static uint8_t pdu[PDU_LEN] __attribute__((aligned(4)));
static uint32_t channel;
static uint32_t radio_was_powered; /* RADIO POWER at the slot start, restored at its end */
#ifdef AIRTAG_ROLLING
static uint32_t active_slot;
static volatile uint32_t wanted_slot; /* written by haystack_poll(), read in the callback */
#endif

static nrf_radio_request_t earliest_request; /* used from the main loop */
static nrf_radio_request_t next_request;     /* used from the signal callback */
static nrf_radio_signal_callback_return_param_t callback_return;

static void select_channel(uint32_t i)
{
    NRF_RADIO->FREQUENCY = channel_freq[i];
    NRF_RADIO->DATAWHITEIV = channel_index[i];
}

#ifdef AIRTAG_ROLLING
/* Apple's real schedule advances by wall-clock time; ours by uptime, since
 * there is no RTC. Only rebuilds the packet when the 15-minute slot has
 * actually changed (called at the start of every ~2 s advert cycle). */
static void airtag_refresh_if_needed(void)
{
    uint32_t slot = wanted_slot; /* no SoftDevice call here: this runs in its interrupt */
    uint8_t addr[HAYSTACK_ADDR_LEN];
    uint32_t i;

    if (slot == active_slot) {
        return;
    }
    active_slot = slot;
    haystack_stats.slot = slot;
    haystack_adv_build(airtag_table[slot], addr, &pdu[PDU_HEADER_LEN + HAYSTACK_ADDR_LEN]);
    for (i = 0U; i < HAYSTACK_ADDR_LEN; i++) {
        pdu[PDU_HEADER_LEN + i] = addr[i];
    }
}
#endif

static void radio_start(void)
{
#ifdef AIRTAG_ROLLING
    airtag_refresh_if_needed();
#endif
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
    NRF_RADIO->PACKETPTR = (uint32_t)pdu;
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

void haystack_poll(void)
{
#ifdef AIRTAG_ROLLING
    wanted_slot = (soft_timer_uptime_ms() / AIRTAG_ROTATE_MS) % AIRTAG_TABLE_COUNT;
#endif
}

uint32_t haystack_init(void)
{
    uint32_t err;

    /* Header: PDU type ADV_NONCONN_IND (2), TxAdd = 1 (random address); then
     * the length of address plus data */
    pdu[0] = 0x42U;
    pdu[1] = (uint8_t)(HAYSTACK_ADDR_LEN + HAYSTACK_PAYLOAD_LEN);
#ifdef AIRTAG_ROLLING
    active_slot = 0xFFFFFFFFUL; /* forces airtag_refresh_if_needed() to build slot 0 */
#else
    {
        uint8_t addr[HAYSTACK_ADDR_LEN];
        uint32_t i;

        haystack_adv_build(adv_key, addr, &pdu[PDU_HEADER_LEN + HAYSTACK_ADDR_LEN]);
        for (i = 0U; i < HAYSTACK_ADDR_LEN; i++) {
            pdu[PDU_HEADER_LEN + i] = addr[i];
        }
    }
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
    default:
        break;
    }
}
