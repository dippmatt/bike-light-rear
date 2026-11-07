/*
 * Light variant: rear (default) or front, selected by the build
 * (make LIGHT=front adds -DLIGHT_FRONT=1). Both run on the same board; this
 * is the only place that branches on the variant for constants. The Mode
 * Cycle (sm.c) and the ANT+ mode map (bike_light.c) are chosen with
 * LIGHT_FRONT next to their single configuration point. See ADR-0009.
 */

#ifndef VARIANT_H
#define VARIANT_H

#ifndef LIGHT_FRONT
#define LIGHT_FRONT 0
#endif

#if LIGHT_FRONT

#define LIGHT_PULSE_BASE  300U /* Standard: 30 % duty */
#define LIGHT_PULSE_PEAK  800U /* Bright: 80 % duty */
#define LIGHT_BASE_PERCENT 30U
#define LIGHT_PEAK_PERCENT 80U
#define SMART_BRAKE_BOOST 0    /* a brighter headlight warns nobody */
#define ANT_LIGHT_TYPE    0U   /* ANT+ light type: headlight */
#define MODEL_NUMBER      0x0002U

#else

#define LIGHT_PULSE_BASE  200U /* normal riding brightness, 20 % duty */
#define LIGHT_PULSE_PEAK  800U /* brake / flash peak brightness */
#define LIGHT_BASE_PERCENT 20U
#define LIGHT_PEAK_PERCENT 80U
#define SMART_BRAKE_BOOST 1
#define ANT_LIGHT_TYPE    2U   /* ANT+ light type: taillight */
#define MODEL_NUMBER      0x0001U

#endif

_Static_assert(LIGHT_BASE_PERCENT * 10U == LIGHT_PULSE_BASE &&
                   LIGHT_PEAK_PERCENT * 10U == LIGHT_PULSE_PEAK,
               "brightness percent must match the pulse width (1000 us PWM period)");

#endif /* VARIANT_H */
