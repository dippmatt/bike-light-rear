/*
 * Vector table and reset handler for the nRF52833 behind a SoftDevice.
 *
 * The MBR/SoftDevice own address 0 and forward interrupts to this table,
 * which therefore has to sit at the start of the application region
 * (0x12000, see linker/rear_light_ant.ld). SVC is handled by the SoftDevice
 * before it reaches this table.
 */

#include <stddef.h>
#include <stdint.h>

extern uint32_t __etext, __data_start, __data_end, __bss_start, __bss_end, __stack_top;

extern void SystemInit(void);
extern int main(void);

void Reset_Handler(void);
void Default_Handler(void);

#define WEAK_HANDLER __attribute__((weak, alias("Default_Handler")))

/* Core exceptions */
void NMI_Handler(void) WEAK_HANDLER;
void HardFault_Handler(void) WEAK_HANDLER;
void MemoryManagement_Handler(void) WEAK_HANDLER;
void BusFault_Handler(void) WEAK_HANDLER;
void UsageFault_Handler(void) WEAK_HANDLER;
void SVC_Handler(void) WEAK_HANDLER;
void DebugMon_Handler(void) WEAK_HANDLER;
void PendSV_Handler(void) WEAK_HANDLER;
void SysTick_Handler(void) WEAK_HANDLER;

/* Peripheral interrupts (nRF52833). Names for the ones the application
 * implements must match the definitions in the other source files. */
void POWER_CLOCK_IRQHandler(void) WEAK_HANDLER;
void RADIO_IRQHandler(void) WEAK_HANDLER;
void UARTE0_UART0_IRQHandler(void) WEAK_HANDLER;
void SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler(void) WEAK_HANDLER;
void SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler(void) WEAK_HANDLER;
void NFCT_IRQHandler(void) WEAK_HANDLER;
void GPIOTE_IRQHandler(void) WEAK_HANDLER;
void SAADC_IRQHandler(void) WEAK_HANDLER;
void TIMER0_IRQHandler(void) WEAK_HANDLER;
void TIMER1_IRQHandler(void) WEAK_HANDLER;
void TIMER2_IRQHandler(void) WEAK_HANDLER;
void RTC0_IRQHandler(void) WEAK_HANDLER;
void TEMP_IRQHandler(void) WEAK_HANDLER;
void RNG_IRQHandler(void) WEAK_HANDLER;
void ECB_IRQHandler(void) WEAK_HANDLER;
void CCM_AAR_IRQHandler(void) WEAK_HANDLER;
void WDT_IRQHandler(void) WEAK_HANDLER;
void RTC1_IRQHandler(void) WEAK_HANDLER;
void QDEC_IRQHandler(void) WEAK_HANDLER;
void COMP_LPCOMP_IRQHandler(void) WEAK_HANDLER;
void SWI0_EGU0_IRQHandler(void) WEAK_HANDLER;
void SWI1_EGU1_IRQHandler(void) WEAK_HANDLER;
void SWI2_EGU2_IRQHandler(void) WEAK_HANDLER; /* SoftDevice event IRQ (SD_EVT_IRQHandler) */
void SWI3_EGU3_IRQHandler(void) WEAK_HANDLER;
void SWI4_EGU4_IRQHandler(void) WEAK_HANDLER;
void SWI5_EGU5_IRQHandler(void) WEAK_HANDLER;
void TIMER3_IRQHandler(void) WEAK_HANDLER;
void TIMER4_IRQHandler(void) WEAK_HANDLER;
void PWM0_IRQHandler(void) WEAK_HANDLER;
void PDM_IRQHandler(void) WEAK_HANDLER;
void MWU_IRQHandler(void) WEAK_HANDLER;
void PWM1_IRQHandler(void) WEAK_HANDLER;
void PWM2_IRQHandler(void) WEAK_HANDLER;
void SPIM2_SPIS2_SPI2_IRQHandler(void) WEAK_HANDLER;
void RTC2_IRQHandler(void) WEAK_HANDLER;
void I2S_IRQHandler(void) WEAK_HANDLER;
void FPU_IRQHandler(void) WEAK_HANDLER;
void USBD_IRQHandler(void) WEAK_HANDLER;
void UARTE1_IRQHandler(void) WEAK_HANDLER;
void PWM3_IRQHandler(void) WEAK_HANDLER;
void SPIM3_IRQHandler(void) WEAK_HANDLER;

typedef void (*vector_fn)(void);

__attribute__((section(".isr_vector"), used))
const vector_fn vector_table[16 + 48] = {
    (vector_fn)(&__stack_top),
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemoryManagement_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    NULL, NULL, NULL, NULL,
    SVC_Handler,
    DebugMon_Handler,
    NULL,
    PendSV_Handler,
    SysTick_Handler,

    POWER_CLOCK_IRQHandler,                          /* 0 */
    RADIO_IRQHandler,                                /* 1 */
    UARTE0_UART0_IRQHandler,                         /* 2 */
    SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler,    /* 3 */
    SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler,    /* 4 */
    NFCT_IRQHandler,                                 /* 5 */
    GPIOTE_IRQHandler,                               /* 6 */
    SAADC_IRQHandler,                                /* 7 */
    TIMER0_IRQHandler,                               /* 8 */
    TIMER1_IRQHandler,                               /* 9 */
    TIMER2_IRQHandler,                               /* 10 */
    RTC0_IRQHandler,                                 /* 11 */
    TEMP_IRQHandler,                                 /* 12 */
    RNG_IRQHandler,                                  /* 13 */
    ECB_IRQHandler,                                  /* 14 */
    CCM_AAR_IRQHandler,                              /* 15 */
    WDT_IRQHandler,                                  /* 16 */
    RTC1_IRQHandler,                                 /* 17 */
    QDEC_IRQHandler,                                 /* 18 */
    COMP_LPCOMP_IRQHandler,                          /* 19 */
    SWI0_EGU0_IRQHandler,                                 /* 20 */
    SWI1_EGU1_IRQHandler,                                 /* 21 */
    SWI2_EGU2_IRQHandler,                                 /* 22 */
    SWI3_EGU3_IRQHandler,                                 /* 23 */
    SWI4_EGU4_IRQHandler,                                 /* 24 */
    SWI5_EGU5_IRQHandler,                                 /* 25 */
    TIMER3_IRQHandler,                               /* 26 */
    TIMER4_IRQHandler,                               /* 27 */
    PWM0_IRQHandler,                                 /* 28 */
    PDM_IRQHandler,                                  /* 29 */
    NULL,                                            /* 30 */
    NULL,                                            /* 31 */
    MWU_IRQHandler,                                  /* 32 */
    PWM1_IRQHandler,                                 /* 33 */
    PWM2_IRQHandler,                                 /* 34 */
    SPIM2_SPIS2_SPI2_IRQHandler,                     /* 35 */
    RTC2_IRQHandler,                                 /* 36 */
    I2S_IRQHandler,                                  /* 37 */
    FPU_IRQHandler,                                  /* 38 */
    USBD_IRQHandler,                                 /* 39 */
    UARTE1_IRQHandler,                               /* 40 */
    NULL, NULL, NULL, NULL,                          /* 41..44 */
    PWM3_IRQHandler,                                 /* 45 */
    NULL,                                            /* 46 */
    SPIM3_IRQHandler,                                /* 47 */
};

/* UICR.NFCPINS = 0xFFFFFFFE: P0.09 and P0.10 are GPIO, not NFC antenna pins.
 * Placed at 0x1000120C by the linker script; a chip erase would otherwise
 * switch them back to NFC and the main LED could not be driven. */
__attribute__((section(".uicr_nfcpins"), used))
const uint32_t uicr_nfcpins = 0xFFFFFFFEUL;

void Reset_Handler(void)
{
    const uint32_t *src = &__etext;
    uint32_t *dst;

    for (dst = &__data_start; dst < &__data_end;) {
        *dst++ = *src++;
    }
    for (dst = &__bss_start; dst < &__bss_end;) {
        *dst++ = 0U;
    }

    SystemInit();
    (void)main();

    for (;;) {
    }
}

void Default_Handler(void)
{
    for (;;) {
    }
}

/* The compiler may emit these even for plain struct copies */
void *memcpy(void *dst, const void *src, size_t n)
{
    uint8_t *d = dst;
    const uint8_t *s = src;

    while (n-- > 0U) {
        *d++ = *s++;
    }
    return dst;
}

void *memset(void *dst, int c, size_t n)
{
    uint8_t *d = dst;

    while (n-- > 0U) {
        *d++ = (uint8_t)c;
    }
    return dst;
}
