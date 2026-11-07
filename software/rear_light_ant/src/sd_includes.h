/*
 * One place that pulls in the ANT SoftDevice API headers.
 *
 * The headers were written against the nRF52832 MDK, which names the
 * software interrupts SWIn_IRQn. The nRF52833 MDK calls them SWIn_EGUn_IRQn.
 * ANT_LICENSE_KEY comes from keys.local.mk via the Makefile (see README.md, "Keys").
 */

#ifndef SD_INCLUDES_H
#define SD_INCLUDES_H

#include "nrf.h"

#ifndef SWI2_IRQn
#define SWI2_IRQn SWI2_EGU2_IRQn
#endif
#ifndef SWI5_IRQn
#define SWI5_IRQn SWI5_EGU5_IRQn
#endif

#include "ant_error.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "nrf_error.h"
#include "nrf_nvic.h"
#include "nrf_sdm.h"
#include "nrf_soc.h"

#endif /* SD_INCLUDES_H */
