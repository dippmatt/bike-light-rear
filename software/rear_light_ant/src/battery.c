/*
 * Battery voltage by single-shot SAADC read, see battery.h.
 */

#include "battery.h"

#include "board.h"
#include "config.h"
#include "soft_timer.h"
#include "sd_includes.h"

#define SAADC_WAIT_LIMIT 200000U
#define BENCH_FAKE_BATTERY_MV 3800U

static int __attribute__((unused)) wait_event(volatile uint32_t *event)
{
    uint32_t guard = SAADC_WAIT_LIMIT;

    while (*event == 0U) {
        if (guard-- == 0U) {
            return -1;
        }
    }
    return 0;
}

static uint32_t last_mv;

uint32_t battery_read_mv(void)
{
#ifdef BENCH_BUILD
    /* Bench build: the board is powered from the debugger without a
     * battery, which would trigger the critical cut-off (BAT-3). Never
     * build a product image with this. */
    last_mv = BENCH_FAKE_BATTERY_MV;
    return last_mv;
#else
    static volatile int16_t sample;
    uint32_t mv = 0U;
    int ok;

    NRF_SAADC->ENABLE = SAADC_ENABLE_ENABLE_Enabled;
    NRF_SAADC->RESOLUTION = SAADC_RESOLUTION_VAL_12bit;
    NRF_SAADC->OVERSAMPLE = SAADC_OVERSAMPLE_OVERSAMPLE_Bypass;
    NRF_SAADC->CH[0].PSELP = SAADC_CH_PSELP_PSELP_AnalogInput5;
    NRF_SAADC->CH[0].PSELN = SAADC_CH_PSELN_PSELN_NC;
    /* Gain 1 with the internal 0.6 V reference: input range 0..0.6 V.
     * 40 us acquisition for the ~91 k divider source impedance. */
    NRF_SAADC->CH[0].CONFIG =
        (SAADC_CH_CONFIG_RESP_Bypass << SAADC_CH_CONFIG_RESP_Pos) |
        (SAADC_CH_CONFIG_RESN_Bypass << SAADC_CH_CONFIG_RESN_Pos) |
        (SAADC_CH_CONFIG_GAIN_Gain1 << SAADC_CH_CONFIG_GAIN_Pos) |
        (SAADC_CH_CONFIG_REFSEL_Internal << SAADC_CH_CONFIG_REFSEL_Pos) |
        (SAADC_CH_CONFIG_TACQ_40us << SAADC_CH_CONFIG_TACQ_Pos) |
        (SAADC_CH_CONFIG_MODE_SE << SAADC_CH_CONFIG_MODE_Pos) |
        (SAADC_CH_CONFIG_BURST_Disabled << SAADC_CH_CONFIG_BURST_Pos);
    NRF_SAADC->RESULT.PTR = (uint32_t)&sample;
    NRF_SAADC->RESULT.MAXCNT = 1U;

    NRF_SAADC->EVENTS_STARTED = 0U;
    NRF_SAADC->EVENTS_END = 0U;
    NRF_SAADC->EVENTS_STOPPED = 0U;

    NRF_SAADC->TASKS_START = 1U;
    ok = wait_event(&NRF_SAADC->EVENTS_STARTED);
    if (ok == 0) {
        NRF_SAADC->TASKS_SAMPLE = 1U;
        ok = wait_event(&NRF_SAADC->EVENTS_END);
    }
    NRF_SAADC->TASKS_STOP = 1U;
    (void)wait_event(&NRF_SAADC->EVENTS_STOPPED);
    NRF_SAADC->ENABLE = SAADC_ENABLE_ENABLE_Disabled;

    if (ok == 0 && sample > 0) {
        /* 0.6 V / 4096 per count, times the divider */
        mv = ((uint32_t)sample * 600U * BATTERY_DIVIDER_RATIO) / 4096U;
    }
    last_mv = mv;
    return mv;
#endif
}

uint32_t battery_last_mv(void)
{
    return last_mv;
}

void battery_tick_start(void)
{
    /* Stale counters from before a sleep must not shorten the time to the
     * critical cut-off */
    battery_logic_reset();
    soft_timer_start(ST_BATTERY, BATTERY_SAMPLE_INTERVAL_MS, BATTERY_SAMPLE_INTERVAL_MS);
}

void battery_tick_stop(void)
{
    soft_timer_stop(ST_BATTERY);
}

enum battery_level battery_sample(void)
{
    return battery_logic_process(battery_read_mv());
}
