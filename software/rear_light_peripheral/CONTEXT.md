# Rear Light Peripheral

Firmware for the bicycle rear light: one button, one main LED, a status LED, USB-C charging and a BLE link to the front light (central). This glossary fixes the words used in the spec, code and manual.

## Power states

**Off**:
The main LED is dark but the controller is awake and reachable over BLE. Entered only by a BLE request, never by the button.
_Avoid_: "OFF" for the sleep step of the mode cycle, standby, idle

**Deep Sleep**:
The lowest power state (System OFF). Nothing runs and BLE is unreachable. Only a button press wakes it.
_Avoid_: Sleep, power-off, shutdown, System OFF (hardware term only)

**Idle Charging**:
The main LED is dark and the controller is awake while Charging. Reached by stepping past the last Active Mode while USB power is present, or by plugging in USB power while in Deep Sleep.
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

## Brightness levels

**Base**:
The normal brightness level of the main LED.
_Avoid_: 50%, base brightness percentages

**Peak**:
The high brightness level of the main LED, used for flashes and braking.
_Avoid_: 80%

## Legacy names

"50%" and "50/80" are leftovers from an earlier 50% Base duty cycle. The duty cycle is now 20%, and these numbers only survive in old documents and commit history. They must not appear in new text.
