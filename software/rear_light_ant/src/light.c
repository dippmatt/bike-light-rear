/*
 * Main LED PWM, see light.h.
 */

#include "light.h"

#include "board.h"
#include "config.h"
#include "sd_includes.h"

#define PWM_POLARITY_ACTIVE_HIGH 0x8000U /* "falling edge": pulse is high for COMPARE counts */
#define PWM_WAIT_LIMIT 100000U

/* PWM sequence value: must live in RAM for EasyDMA */
static volatile uint16_t pwm_value;
static bool pwm_running;

/* Blank gate: light_set() always records the requested level, but the PWM
 * only follows it while the output is not blanked */
static uint32_t requested_pulse_usec;
static bool blanked;

static void apply_pulse(uint32_t pulse_us)
{
    if (pulse_us == 0U) {
        if (pwm_running) {
            uint32_t guard = PWM_WAIT_LIMIT;

            NRF_PWM0->EVENTS_STOPPED = 0U;
            NRF_PWM0->TASKS_STOP = 1U;
            while (NRF_PWM0->EVENTS_STOPPED == 0U && guard-- > 0U) {
            }
            pwm_running = false;
        }
        NRF_P0->OUTCLR = 1UL << PIN_MAIN_LED;
        return;
    }

    if (pulse_us > LIGHT_PWM_PERIOD_USEC) {
        pulse_us = LIGHT_PWM_PERIOD_USEC;
    }

    /* 1 MHz PWM clock: one count is one microsecond */
    pwm_value = (uint16_t)(PWM_POLARITY_ACTIVE_HIGH | pulse_us);
    NRF_PWM0->EVENTS_SEQEND[0] = 0U;
    NRF_PWM0->TASKS_SEQSTART[0] = 1U;
    pwm_running = true;
}

void light_init(void)
{
    NRF_P0->DIRSET = 1UL << PIN_MAIN_LED;
    NRF_P0->OUTCLR = 1UL << PIN_MAIN_LED;

    NRF_PWM0->PSEL.OUT[0] = PIN_MAIN_LED; /* port 0, pin 9 */
    NRF_PWM0->PSEL.OUT[1] = 0xFFFFFFFFUL;
    NRF_PWM0->PSEL.OUT[2] = 0xFFFFFFFFUL;
    NRF_PWM0->PSEL.OUT[3] = 0xFFFFFFFFUL;
    NRF_PWM0->MODE = PWM_MODE_UPDOWN_Up;
    NRF_PWM0->PRESCALER = PWM_PRESCALER_PRESCALER_DIV_16; /* 16 MHz / 16 = 1 MHz */
    NRF_PWM0->COUNTERTOP = LIGHT_PWM_PERIOD_USEC;
    NRF_PWM0->LOOP = 0U;
    NRF_PWM0->DECODER = (PWM_DECODER_LOAD_Common << PWM_DECODER_LOAD_Pos) |
                        (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);
    NRF_PWM0->SEQ[0].PTR = (uint32_t)&pwm_value;
    NRF_PWM0->SEQ[0].CNT = 1U;
    NRF_PWM0->SEQ[0].REFRESH = 0U;
    NRF_PWM0->SEQ[0].ENDDELAY = 0U;
    NRF_PWM0->ENABLE = PWM_ENABLE_ENABLE_Enabled;

    requested_pulse_usec = LIGHT_PULSE_OFF;
    blanked = false;
}

void light_set(uint32_t pulse_usec)
{
    requested_pulse_usec = pulse_usec;
    if (!blanked) {
        apply_pulse(pulse_usec);
    }
}

void light_blank_begin(void)
{
    blanked = true;
    apply_pulse(LIGHT_PULSE_OFF);
}

void light_blank_end(void)
{
    blanked = false;
    apply_pulse(requested_pulse_usec);
}
