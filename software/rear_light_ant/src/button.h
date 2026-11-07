/*
 * Button on P1.00 (active low, pull-up), SPEC section 3.
 *
 * The press edge is detected by the GPIO sense interrupt (no GPIOTE channel
 * and no current while the CPU sleeps). The main loop then samples the pin
 * every SAMPLE_INTERVAL_MS: released before LONG_PRESS_SAMPLES samples is a
 * short press, still held at the last sample a long press. After a press the
 * sense interrupt stays off until the contact has been seen released.
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>

/** Configure the pin and the interrupt; the button stays disarmed. @return 0 on success */
int button_init(void);

/**
 * Arm the press detection (boot, after wake). A button that is still held
 * produces no event; the next press edge counts as a new press (BOOT-4).
 * @return 0 on success
 */
int button_enable(void);
int button_reenable(void);

/** Arm the one-shot wake interrupt for Deep Sleep. @return 0 on success */
int button_arm_wake(void);

bool button_is_pressed(void);

/** EVT_BUTTON_EDGE: the sense interrupt fired */
void button_on_edge(void);

/** EVT_BUTTON_SAMPLE: the press-length sampling timer fired */
void button_on_sample(void);

#endif /* BUTTON_H */
