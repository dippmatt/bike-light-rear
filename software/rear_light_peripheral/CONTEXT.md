# Rear Light Peripheral

Firmware for the bicycle rear light: one button, one main LED, a status LED, USB-C charging and a BLE link to the front light (central). This glossary fixes the words used in the spec, code and manual.

## Power states

**Off**:
The main LED is dark but the controller is awake and reachable over BLE. Entered only by a BLE request, never by the button.
_Avoid_: "OFF" for the sleep step of the mode cycle, standby, idle

**Deep Sleep**:
The lowest power state. Outputs, sensors, battery sampling and BLE advertising are stopped and the CPU idles (System ON, not System OFF, see ADR-0004). BLE is unreachable. Only a button press or USB power appearing wakes it, without a reset.
_Avoid_: Sleep, power-off, shutdown, System OFF (a different hardware mode that the firmware no longer uses)

**Idle Charging**:
The main LED is dark and the controller is awake while Charging. Reached instead of Deep Sleep whenever the light is powered off (short press past the last Active Mode, long press, auto-off, critical battery) while USB power is present, or by plugging in USB power while in Deep Sleep. Removing USB power in Idle Charging enters Deep Sleep.
_Avoid_: Charging mode, charge mode

## Light modes

**Steady**:
The main LED shows Base brightness continuously.
_Avoid_: 50%, 50% Continuous, Mode 2

**Flash**:
Base brightness with a periodic double flash to Peak brightness.
_Avoid_: 50/80 Flash, High Visibility

**Smart**:
Brightness is chosen automatically from braking, ambient light and motion.
_Avoid_: Smart Mode (acceptable in prose), Adaptive

## Mode cycle

**Active Mode**:
A member of the mode cycle that runs the light: Steady, Flash or Smart. Smart counts even when its LED happens to be dark.
_Avoid_: lit mode, light mode (too broad: includes Off)

**Mode Cycle**:
The ordered list of Active Modes that short presses step through. Its order is a single configuration point, and no other behaviour may depend on a specific order.
_Avoid_: mode ring, mode sequence

**Selection Window**:
The period right after the light goes from an inactive state (Deep Sleep, Off, Idle Charging) to an Active Mode, in which short presses still step through the Mode Cycle. Changing between Active Modes never starts or restarts it. Its length is about 15 to 20 seconds, because it is counted in battery ticks.
_Avoid_: mode timeout, cycle timeout

**Charging**:
The condition that USB power is present. The charger is autonomous hardware, so the firmware treats USB power present as charging in every awake state, and has no charge-complete knowledge.
_Avoid_: USB connected (for the condition), charge state

**Blank**:
The short window (about 120 ms) in which Smart switches the lit main LED off to measure ambient light without the LED's own light. The brightness requested by the state machine is restored right after.
_Avoid_: dropout, dark pulse, LED off-on cycle

## Brightness levels

**Base**:
The normal brightness level of the main LED.
_Avoid_: 50%, base brightness percentages

**Peak**:
The high brightness level of the main LED, used for flashes and braking.
_Avoid_: 80%

## Legacy names

"50%" and "50/80" are leftovers from an earlier 50% Base duty cycle. The duty cycle is now 20%, and these numbers only survive in old documents and commit history. They must not appear in new text.
