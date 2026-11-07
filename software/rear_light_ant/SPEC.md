# Rear Light (ANT+) - Firmware Specification

Behavioural requirements of the rear light firmware. Terms are defined in [CONTEXT.md](./CONTEXT.md). Numeric values are **not** repeated here: the code constant named in `[...]` is the single source of truth (all of them live in `src/config.h` unless noted). Hard-to-reverse decisions are recorded in [docs/adr](./docs/adr).

This firmware replaces the Zephyr/BLE firmware in `../rear_light_peripheral`. Sections 1 to 9 and 11 carry its requirements over (IDs unchanged so external test lists keep working); section 10 replaces the BLE section with the ANT+ Bike Lights profile (ADR-0005). The ANT+ page layouts are defined by the profile document D00001621, not repeated here.

Each requirement has an ID so external test lists can reference it.

## 1. System and states

- **SYS-1** The firmware shall have exactly five states: Off, Steady, Flash, Smart and Idle Charging. Their numeric values (0..4 in that order) are internal; the ANT+ mode numbers are defined only by the mode map (ANT-3).
- **SYS-2** The Mode Cycle shall be a single configuration point listing the Active Modes in short-press order (currently Smart, Steady, Flash). No other behaviour shall depend on that order or on a specific mode being first or last.
- **SYS-3** All inputs (button, controller commands, battery tick, USB edges, sensor detections, flash timer) shall be delivered as events to one queue consumed by the main loop. Only the main loop shall change the state or choose the main LED level. The one exception is the blank (SMART-12), where the sensor module may switch the LED off temporarily; the state machine's requested level is kept and restored. `[APP_EVENT_QUEUE_DEPTH]`
- **SYS-4** If the event queue is full, the new event shall be dropped and counted in the debug counters. No retry is made.
- **SYS-5** If initialization of the main LED, the status LED or the button fails, the firmware shall perform a software reset instead of returning from `main()`. Failure of the battery ADC, the sensors or the ANT SoftDevice shall not stop startup: without the SoftDevice the light works locally and the failure code is kept in the debug counters. See ADR-0003.
- **SYS-6** The firmware shall not use a watchdog. A hang is not recovered. A SoftDevice fault resets the chip (ANT-13). (Possible future feature.)

## 2. Boot

- **BOOT-1** At boot the firmware shall enter Idle Charging if the reset cause is a power-on reset and USB power is present. A power-on reset is a reset with no reset-reason flag set.
- **BOOT-2** For every other reset cause (power-on reset without USB power, brownout, watchdog, software or pin reset, or a non-power-on reset with USB power) the firmware shall enter the first Active Mode of the Mode Cycle.
- **BOOT-3** Plugging in USB power while in Deep Sleep shall wake the device into Idle Charging. The wake is detected by the USB poll (USB-1), so it may take up to one poll interval. It is a wake, not a reset.
- **BOOT-4** The press that wakes the light from Deep Sleep shall only wake it (the device then behaves as after a boot: first Active Mode, Selection Window open). It shall produce no short or long press event. A button still held after waking produces no event, and the next press edge counts as a new press.

## 3. Button

- **BTN-1** A press released before 1 s shall be a short press. It acts on release. `[LONG_PRESS_SAMPLES, SAMPLE_INTERVAL_MS]`
- **BTN-2** A press held for 1 s or longer shall be a long press. It acts at the 1 s mark without waiting for release.
- **BTN-3** Press edges closer together than the debounce time shall be ignored. `[DEBOUNCE_MS]`
- **BTN-4** A short press shall act as follows:
  - Off: enter the first Active Mode.
  - Idle Charging: enter the first Active Mode.
  - An Active Mode that is not the last in the Mode Cycle, inside the Selection Window (BTN-8): enter the next Active Mode.
  - The last Active Mode, or any Active Mode after the Selection Window has closed: enter Idle Charging if USB power is present, otherwise enter Deep Sleep.
- **BTN-5** A long press in any state shall power the light off: enter Idle Charging if USB power is present (so the status LED keeps showing Charging), otherwise enter Deep Sleep. A long press in Idle Charging with USB power present shall do nothing.
- **BTN-6** Before entering Deep Sleep the firmware shall wait until the button has read released for several polls in a row (contact bounce on release has settled), up to a limit, so that a still-held or bouncing button does not wake the device immediately. If the limit is exceeded it proceeds anyway. `[SLEEP_RELEASE_WAIT_MS, SLEEP_RELEASE_POLL_MS, SLEEP_RELEASE_STABLE_POLLS]`
- **BTN-7** If the Deep Sleep wake interrupt cannot be armed, the firmware shall stay awake in Off with the button working instead of sleeping.
- **BTN-8** Every transition from an inactive state (Deep Sleep, Off, Idle Charging) to an Active Mode shall open the Selection Window, whether caused by the button, a boot or reset, or a controller request. Changes between Active Modes, by button or by controller, shall neither open nor restart it. The window shall be counted in battery ticks (no dedicated timer), so its length is between (N-1) and N tick periods, about 15 to 20 s. `[MODE_SELECT_WINDOW_BATTERY_TICKS, BATTERY_SAMPLE_INTERVAL_MS]`
- **BTN-9** Closing the Selection Window shall have no visible effect. Only the effect of the next short press changes (BTN-4). USB power is evaluated at the time of that press. A long press is unaffected.

## 4. Power states

- **PWR-1** In Deep Sleep the main LED and the status LED shall be off, sensor sampling and battery sampling shall be stopped, and the ANT channel shall be closed (the light transmits nothing and is unreachable). Deep Sleep is the CPU idling in System ON (ADR-0004), not System OFF, so waking causes no reset. Only the button (or USB power appearing, see BOOT-3) wakes the device.
- **PWR-2** Off shall be entered only by a controller request (ANT-7). It shall have no timeout and shall not enter Deep Sleep on its own.
- **PWR-3** In Off and Idle Charging the main LED shall be off.
- **PWR-4** Every power-off (long press, Smart stationary auto-off, critical battery, and the end of the short-press cycle, BTN-4) shall enter Idle Charging if USB power is present, otherwise Deep Sleep. Removing USB power in Idle Charging shall enter Deep Sleep (USB-3). USB presence is taken from the USB poll, so it may be one poll interval old.

## 5. Light output

- **LED-1** Steady shall show Base continuously. `[LIGHT_PULSE_BASE]`
- **LED-2** Flash shall show Base, with a double flash to Peak every cycle: Peak for one step, Base for one step, Peak for one step, Base for the rest of the cycle. The first double flash occurs one cycle after entering Flash. `[FLASH_CYCLE_MS, FLASH_STEP_MS, FLASH_REST_MS, LIGHT_PULSE_PEAK]`
- **LED-3** The PWM period shall be fixed. `[LIGHT_PWM_PERIOD_USEC]`
- **LED-4** Leaving Flash shall stop the flash timer. A flash step event arriving in any other state shall be ignored.

## 6. Smart

- **SMART-1** On entering Smart the main LED shall be off, and the braking and dark flags shall be cleared. Sensor sampling shall start. Sampling shall stop when Smart is left. The accelerometer and the light sensor shall be sampled on independent schedules. `[ACCEL_SAMPLING_INTERVAL_MS, LIGHT_SAMPLING_INTERVAL_MS, SENSOR_TICK_MS]`
- **SMART-2** Brightness in Smart shall be chosen by priority: braking gives Peak, otherwise dark gives Base, otherwise off.
- **SMART-3** Braking is judged on the filtered Z acceleration (SMART-8, SMART-9). The sensor is mounted with Z along the direction of travel and positive Z means decelerating and negative Z means accelerating. Braking shall start when the filtered value stays above the threshold for the start confirm time, and end when it stays at or below the same threshold for the stop confirm time plus the hold time, so the light keeps shining for the hold time after the braking phase has ended. Braking again within that time keeps the light on without a gap. The threshold is +5.0 m/s^2 (not g). `[BRAKING_ACCEL_THRESHOLD_MILLI, BRAKING_START_CONFIRM_MS, BRAKING_STOP_CONFIRM_MS, BRAKING_HOLD_MS]`
- **SMART-4** Darkness starts after consecutive samples below the ambient threshold and ends after consecutive samples at or above the ambient threshold plus a fixed off offset. The offset only prevents flicker around the toggle point. `[AMBIENT_THRESHOLD_MILLILUX, AMBIENT_OFF_OFFSET_MILLILUX, AMBIENT_SAMPLES_REQUIRED]`
- **SMART-5** After the stationary timeout of uninterrupted samples with acceleration magnitude within the tolerance band around 1g, the firmware shall power off (auto-off, PWR-4). Any sample outside the band resets the count. A failed accelerometer read neither counts nor resets. See ADR-0002. `[STATIONARY_TIMEOUT_MS, STATIONARY_MIN_MAGNITUDE_SQ, STATIONARY_MAX_MAGNITUDE_SQ]`
- **SMART-6** Braking, darkness and auto-off events arriving in any state other than Smart shall be ignored.
- **SMART-7** If a sensor failed initialization, Smart shall continue with the remaining one. No light sensor means the LED stays off. No accelerometer means no braking and no auto-off.
- **SMART-8** The filtered Z acceleration shall be the raw Z acceleration minus a slow baseline (removes the gravity component from mounting tilt and slope). The baseline shall be seeded from the first accelerometer sample after entering Smart, follow Z with a first-order low-pass, and be frozen while braking is active or a braking start is being confirmed. `[BRAKING_BASELINE_TAU_MS]`
- **SMART-9** The baseline-corrected Z acceleration shall be averaged over a short window before it is compared to the threshold. A window of one sample disables the smoothing. `[BRAKING_SMOOTHING_MS]`
- **SMART-10** The accelerometer output data rate shall be set explicitly at start-up (not left at the chip default) and shall not be lower than the polling rate. `[ACCEL_ODR_HZ]`
- **SMART-11** Detection times (braking confirm, smoothing, baseline, stationary timeout) shall be defined in milliseconds, and the sample counts derived from them rounded up, so changing a sampling interval does not change a detection time. Ambient light detection is defined in light sensor samples. `[MS_TO_SAMPLES]`
- **SMART-12** Ambient light shall be measured without the main LED's own light. While the main LED is lit at Base (dark), the measurement shall be taken with the main LED blanked (off) for one sensor conversion and the LED shall be restored right after the result is read. Blanked measurements are taken at a fixed long interval, so the LED goes dark only briefly and rarely. The first blanked reading that reaches the off point is confirmed after one light sampling interval instead. While the LED is off, the sensor is read without blanking at the light sampling interval. `[LIGHT_BLANK_INTERVAL_MS, AMBIENT_BLANK_SETTLE_MS, LIGHT_SAMPLING_INTERVAL_MS]`
- **SMART-13** No ambient light measurement shall be taken while braking (the LED is lit at Peak), nor while a braking start is being confirmed. The first measurement after braking ended is due one light sampling interval later, so the sensor's last conversion is free of the LED's light.
- **SMART-14** The light sensor conversion time shall be set explicitly to 100 ms (the chip default is 800 ms, which a short blank could not influence). It shall be minimal, because the LED must stay dark for the whole conversion (datasheet: 90 to 110 ms plus up to 10 ms for the first auto-range step). If the sensor cannot be configured, it shall be treated as failed (SMART-7). `[OPT3001_CONFIG_CONTINUOUS, OPT3001_CONFIG_SINGLE_SHOT, AMBIENT_BLANK_TIMEOUT_MS]`
- **SMART-15** A blank shall always end, also on a sensor error, a timeout or when sampling stops. Any level requested by the state machine during a blank shall take effect when the blank ends, so a stale level is never restored (e.g. Off after Deep Sleep). The blank is a sequence of timer steps in the main loop, so other events (including accelerometer polls) are still handled while it runs.

## 7. USB power and charging

- **USB-1** USB power presence shall be polled, and an event posted only on a change. A newly plugged or unplugged cable may therefore be noticed up to one poll interval late. `[USB_POLL_INTERVAL_MS]`
- **USB-2** Plugging in USB power shall never change the state while the device is awake. Only the status LED changes. The one exception is Deep Sleep (BOOT-3).
- **USB-3** Removing USB power while in Idle Charging shall enter Deep Sleep. In any other state it shall only change the status LED.
- **USB-4** The firmware shall treat USB power present as Charging in every awake state. It has no knowledge of charge completion, and charging itself is done by autonomous hardware.

## 8. Battery

- **BAT-1** In every awake state the battery voltage shall be sampled periodically. `[BATTERY_SAMPLE_INTERVAL_MS]`
- **BAT-2** A failed reading shall count as 0 mV, so a battery that cannot be measured is treated as empty.
- **BAT-3** After consecutive samples below the critical voltage the firmware shall power off (PWR-4), in every state. `[BATTERY_CRITICAL_MV, BATTERY_CONSECUTIVE]`
- **BAT-4** After consecutive samples below the low voltage the battery shall be Low. After consecutive samples at or above it, the battery shall be OK again. Entering and leaving Low use the same voltage. `[BATTERY_LOW_MV, BATTERY_CONSECUTIVE]`
- **BAT-5** Low battery shall not change the main LED. Only the status LED and the critical cut-off (BAT-3) react.
- **BAT-6** Battery readings while USB power is present are not trustworthy and no special handling exists.
- **BAT-7** Waking from Deep Sleep with a critical battery shall turn the light on until the critical cut-off triggers again, as for any other wake.

## 9. Status LED

- **STAT-1** The status LED shall show, in priority order:
  1. Low battery: slow blink. `[BLINK_INTERVAL_LOW_BATTERY_MS]`
  2. Charging: fast blink. `[BLINK_INTERVAL_CHARGING_MS]`
  3. Any Active Mode: solid on.
  4. Off while awake (set by a controller, the light is dark but reachable): a short pulse every few seconds (heartbeat), so it can be told from Deep Sleep. `[HEARTBEAT_PERIOD_MS, HEARTBEAT_ON_MS]`
  5. Otherwise (Deep Sleep): off.
- **STAT-2** The status LED is visible to the rider. This is the only low-battery warning, and the main LED is unaffected until the critical cut-off.

## 10. ANT+ Bike Lights

The light is an ANT+ bike light (profile D00001621 Rev 2.0): a bidirectional master that a controller (bike computer) finds, reads and commands. Light state and capabilities are broadcast, commands arrive as acknowledged data.

- **ANT-1** Whenever the device is awake it shall transmit as an ANT+ bike light master: device type, RF frequency, channel period and transmission type as defined in the profile, and a device number derived from the chip's factory ID that is never zero. `[ANT_DEVICE_TYPE_BIKE_LIGHT, ANT_RF_FREQUENCY, ANT_CHANNEL_PERIOD, BL_TRANSMISSION_TYPE]` If the channel cannot be opened, opening is retried at every battery tick while awake. The ANT+ network key is not in the repository; the developer supplies it (README.md, "Keys").
- **ANT-2** The channel shall be closed when entering Deep Sleep and opened again when leaving it (PWR-1). A controller therefore cannot reach a sleeping light. Off and Idle Charging stay awake and reachable.
- **ANT-3** One table (`mode_map` in `src/bike_light.c`) shall map the ANT+ standard modes to the firmware states: Off to mode 0, Steady to mode 5 (steady 0-20 %, Base is a 20 % duty), Flash to mode 6 (slow flash), Smart to mode 9 (auto). Idle Charging is reported as Off. Page 2 shall list exactly the modes of this table, and the number of modes it announces shall equal the number of standard modes in its bit field (a controller assumes any difference to be custom modes).
- **ANT-4** Page 1 shall report light index 0 (unconnected) or 1 (connected, ANT-6), the light type tail light, the battery status (ANT-12), the sequence number of the last command (0xFF until one was received), the current mode and its intensity (Steady 20 %, Smart auto, other 0xFF or 0).
- **ANT-5** After every event (a state change, a command, a battery status change) the transmission pattern shall restart with page 1 three times, then continue with pages 2, 80 and 81 (and 18 while connected). Page 19 (battery percent) and any other supported page shall be sent on a page 70 request, repeated a limited number of times. `[EVENT_PAGE1_REPEATS, MAX_REQUEST_REPEATS in src/bike_light.c]`
- **ANT-6** A controller forms a network even for a single light: the connect command (page 33) with light index 1 makes this light the main light. It shall then report light index 1 in its pages, send its channel ID (page 18), echo the controller ID as the sequence number, apply the optional light state setting (off, flashing, steady), and accept settings addressed to light index 0 or 1. Connect commands for secondary lights (index 2 to 63) are ignored, because the shared channel is not implemented. The disconnect command (page 32) for index 0 or 1 returns the light to unconnected.
- **ANT-7** Light settings (page 34) shall request a state: beam off requests Off, a supported mode requests the mapped state, a beam on command without mode while Off requests the last Active Mode, an unsupported mode is ignored. The sequence number of every command that is addressed to a light is reported (ANT-4), also if the command is ignored. Valid requests are Off, Steady, Flash and Smart. Idle Charging cannot be requested because only USB power sets it. Requests are delivered to the state machine as events (SYS-3).
- **ANT-8** A request shall be ignored if it equals the current state, or arrives within the stable window after the last state change (including boot). `[REMOTE_STABLE_WINDOW_MS]`
- **ANT-9** A request for Steady, Flash or Smart shall be applied in Idle Charging.
- **ANT-10** A request for Off shall keep the device awake and reachable.
- **ANT-11** If no message from a controller has been received for the silence timeout, a connected light shall return to unconnected (light index 0). The main LED shall not change because of this timeout. The timeout is counted in battery ticks. `[ANT_SILENCE_TIMEOUT_BATTERY_TICKS]`
- **ANT-12** The battery status in page 1 shall be Charging while USB power is present, otherwise derived from the battery voltage (new, good, ok, low, critical), invalid if the voltage cannot be measured. Page 19 shall report the battery percentage. `[BATTERY_STATUS_*_MV]`
- **ANT-13** If the SoftDevice reports a fault the chip shall reset (the boot policy then restores the light, BOOT-2). The fault details are kept in the debug counters.
- **ANT-14** Controller commands are not authenticated. Any ANT+ controller in range can change the state. See ADR-0001.
- **ANT-15** Debug counters (transmitted events, received messages, ANT events, the last received messages, SoftDevice errors, dropped events, resets) shall be readable over SWD without a console.

## 11. Hardware assumptions

- **HW-1** The charger is autonomous hardware that charges in Deep Sleep and with a flat battery.
- **HW-2** USB power appearing is detected by software polling of the VBUS status while in Deep Sleep. No hardware reset or wake from it is assumed.
- **HW-3** The accelerometer is mounted with Z along the direction of travel, with +Z pointing backward (against the direction of travel), so braking reads positive and accelerating reads negative.
- **HW-4** The status LED is visible to the rider.
- **HW-5** The board has no 32.768 kHz crystal; the low-frequency clock is the internal RC oscillator (ADR-0006).
