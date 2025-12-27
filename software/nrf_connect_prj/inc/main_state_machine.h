#ifndef MAIN_STATE_MACHINE_H
#define MAIN_STATE_MACHINE_H

/**
 * @brief Initialize the main state machine
 * 
 * Sets up the state machine and connects button and LED control.
 * 
 * @return 0 on success, negative error code on failure
 */
int main_state_machine_init(void);

/**
 * @brief Handle button press event
 * 
 * Called by button module when a button press is detected.
 * This function cycles through LED brightness states.
 */
void main_state_machine_on_button_press(void);

#endif /* MAIN_STATE_MACHINE_H */

