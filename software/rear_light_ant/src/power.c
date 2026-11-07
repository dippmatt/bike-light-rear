/*
 * USB power detection and reset cause, see power.h.
 */

#include "power.h"

#include "app_events.h"
#include "config.h"
#include "sd_includes.h"
#include "soft_timer.h"

static bool usb_present_prev;

uint32_t power_take_reset_reason(void)
{
    uint32_t reason = NRF_POWER->RESETREAS;

    NRF_POWER->RESETREAS = reason; /* write one to clear */
    return reason;
}

bool power_reset_is_power_on(uint32_t reason)
{
    return reason == 0U;
}

bool power_usb_present(void)
{
    return (NRF_POWER->USBREGSTATUS & POWER_USBREGSTATUS_VBUSDETECT_Msk) != 0U;
}

void power_usb_poll_start(void)
{
    usb_present_prev = power_usb_present();
    soft_timer_start(ST_USB, USB_POLL_INTERVAL_MS, USB_POLL_INTERVAL_MS);
}

void power_usb_poll(void)
{
    bool present = power_usb_present();

    if (present != usb_present_prev) {
        usb_present_prev = present;
        (void)app_event_post(present ? EVT_USB_CONNECTED : EVT_USB_DISCONNECTED, 0U);
    }
}
