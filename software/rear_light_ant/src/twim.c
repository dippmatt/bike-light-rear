/*
 * Blocking I2C master, see twim.h.
 */

#include "twim.h"

#include "board.h"
#include "sd_includes.h"

#define TWIM_WAIT_LIMIT 200000U

static void configure_pin(uint32_t pin)
{
    /* Open-drain style drive, external pull-ups on the board (TWIM
     * requirement: input connected, S0D1, no internal pull) */
    NRF_P0->PIN_CNF[pin] =
        (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
        (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
        (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
        (GPIO_PIN_CNF_DRIVE_S0D1 << GPIO_PIN_CNF_DRIVE_Pos) |
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
}

void twim_enable(void)
{
    configure_pin(PIN_I2C_SCL);
    configure_pin(PIN_I2C_SDA);

    NRF_TWIM0->PSEL.SCL = PIN_I2C_SCL;
    NRF_TWIM0->PSEL.SDA = PIN_I2C_SDA;
    NRF_TWIM0->FREQUENCY = TWIM_FREQUENCY_FREQUENCY_K400;
    NRF_TWIM0->SHORTS = 0U;
    NRF_TWIM0->ENABLE = TWIM_ENABLE_ENABLE_Enabled;
}

void twim_disable(void)
{
    NRF_TWIM0->ENABLE = TWIM_ENABLE_ENABLE_Disabled;
    NRF_TWIM0->PSEL.SCL = 0xFFFFFFFFUL;
    NRF_TWIM0->PSEL.SDA = 0xFFFFFFFFUL;
}

/* Wait until the transfer stopped or failed; always leaves the bus stopped */
static int finish(void)
{
    uint32_t guard = TWIM_WAIT_LIMIT;
    int result = 0;

    while (NRF_TWIM0->EVENTS_STOPPED == 0U && NRF_TWIM0->EVENTS_ERROR == 0U) {
        if (guard-- == 0U) {
            result = -1;
            break;
        }
    }

    if (NRF_TWIM0->EVENTS_ERROR != 0U || result != 0) {
        result = -1;
        NRF_TWIM0->EVENTS_STOPPED = 0U;
        NRF_TWIM0->TASKS_STOP = 1U;
        guard = TWIM_WAIT_LIMIT;
        while (NRF_TWIM0->EVENTS_STOPPED == 0U && guard-- > 0U) {
        }
    }

    NRF_TWIM0->ERRORSRC = TWIM_ERRORSRC_ANACK_Msk | TWIM_ERRORSRC_DNACK_Msk |
                          TWIM_ERRORSRC_OVERRUN_Msk;
    return result;
}

static void clear_events(void)
{
    NRF_TWIM0->EVENTS_STOPPED = 0U;
    NRF_TWIM0->EVENTS_ERROR = 0U;
    NRF_TWIM0->EVENTS_LASTTX = 0U;
    NRF_TWIM0->EVENTS_LASTRX = 0U;
}

int twim_write(uint8_t address, const uint8_t *data, uint8_t length)
{
    NRF_TWIM0->ADDRESS = address;
    NRF_TWIM0->TXD.PTR = (uint32_t)data;
    NRF_TWIM0->TXD.MAXCNT = length;
    NRF_TWIM0->SHORTS = TWIM_SHORTS_LASTTX_STOP_Msk;
    clear_events();
    NRF_TWIM0->TASKS_STARTTX = 1U;
    return finish();
}

int twim_write_read(uint8_t address, const uint8_t *tx, uint8_t tx_length, uint8_t *rx,
                    uint8_t rx_length)
{
    NRF_TWIM0->ADDRESS = address;
    NRF_TWIM0->TXD.PTR = (uint32_t)tx;
    NRF_TWIM0->TXD.MAXCNT = tx_length;
    NRF_TWIM0->RXD.PTR = (uint32_t)rx;
    NRF_TWIM0->RXD.MAXCNT = rx_length;
    NRF_TWIM0->SHORTS = TWIM_SHORTS_LASTTX_STARTRX_Msk | TWIM_SHORTS_LASTRX_STOP_Msk;
    clear_events();
    NRF_TWIM0->TASKS_STARTTX = 1U;
    return finish();
}
