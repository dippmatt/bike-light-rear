/*
 * Application linker script for the nRF52833 behind the ANT S212 7.0.1
 * SoftDevice.
 *
 *   0x00000000 MBR (4 kB) + SoftDevice (68 kB), flashed separately
 *   0x00012000 application: its vector table must be the first thing here
 *
 * RAM: the SoftDevice owns the first 0xB80 bytes (2.94 kB); a little margin
 * is kept. The ANT stack's own buffer is a normal static array in .bss.
 */

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00012000, LENGTH = 0x0006E000
    RAM   (rwx) : ORIGIN = 0x20000C00, LENGTH = 0x0001F400
    UICR_NFCPINS (r) : ORIGIN = 0x1000120C, LENGTH = 4
}

/* The SoftDevice runs on the application stack: it needs up to 1.75 kB
 * (0x700) on top of the application's own worst case (release notes). */
STACK_SIZE = 0x1000;

ENTRY(Reset_Handler)

SECTIONS
{
    .text :
    {
        KEEP(*(.isr_vector))
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    __etext = .;

    .data : AT(__etext)
    {
        . = ALIGN(4);
        __data_start = .;
        *(.data*)
        . = ALIGN(4);
        __data_end = .;
    } > RAM

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > RAM

    .stack (NOLOAD) :
    {
        . = ALIGN(8);
        __stack_limit = .;
        . += STACK_SIZE;
        . = ALIGN(8);
        __stack_top = .;
    } > RAM

    /* UICR.NFCPINS: P0.09/P0.10 as GPIO (main LED is on P0.09) */
    .uicr_nfcpins :
    {
        KEEP(*(.uicr_nfcpins))
    } > UICR_NFCPINS

    /DISCARD/ :
    {
        *(.ARM.exidx*)
        *(.ARM.extab*)
        *(.comment)
    }
}

ASSERT(__stack_top <= ORIGIN(RAM) + LENGTH(RAM), "RAM overflow (data + bss + stack)")
