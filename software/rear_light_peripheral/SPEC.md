# Rear Light Peripheral - Firmware Specification

Behavioural requirements of the rear light firmware. Terms are defined in [CONTEXT.md](./CONTEXT.md). Numeric values are **not** repeated here: the code constant named in `[...]` is the single source of truth. Hard-to-reverse decisions are recorded in [docs/adr](./docs/adr).

Each requirement has an ID so external test lists can reference it.

## 1. System and states

- **SYS-1** The firmware shall have exactly five states: Off, Steady, Flash, Smart and Idle Charging. Their numeric values (0..4 in that order) are part of the BLE contract and shall not change.
- **SYS-2** The Mode Cycle shall be a single configuration point listing the Active Modes in short-press order (currently Smart, Steady, Flash). No other behaviour shall depend on that order or on a specific mode being first or last.
- **SYS-3** All inputs (button, BLE, battery tick, USB edges, sensor detections, flash timer) shall be delivered as events to one queue consumed by a single state machine thread. Only that thread shall change the state or the main LED output. `[APP_EVENT_QUEUE_DEPTH]`
- **SYS-4** If the event queue is full, the new event shall be dropped and a message logged. No retry is made.
- **SYS-5** If initialization of the main LED, the status LED or the button fails, the firmware shall perform a software reset instead of returning from `main()`. Failure of the battery ADC, the sensors or BLE initialization shall not stop startup. See ADR-0003.
- **SYS-6** The firmware shall not use a watchdog. A hang is not recovered. (Possible future feature.)

## 2. Boot

- **BOOT-1** At boot the firmware shall enter Idle Charging if the reset cause is a power-on reset and USB power is present.
- **BOOT-2** For every other reset cause (button wake from Deep Sleep, power-on reset without USB power, brownout, watchdog, software or pin reset, or a non-power-on reset with USB power) the firmware shall enter the first Active Mode of the Mode Cycle.
- **BOOT-3** Plugging in USB power while in Deep Sleep resets the MCU in hardware, so by BOOT-1 the light shall come up in Idle Charging.
- **BOOT-4** The press that wakes the light from Deep Sleep shall only wake it. It shall produce no short or long press event. A button still held after boot produces no event, and the next press edge counts as a new press.

## 3. Button

- **BTN-1** A press released before 1 s shall be a short press. It acts on release. `[LONG_PRESS_SAMPLES, SAMPLE_INTERVAL_MS]`
- **BTN-2** A press held for 1 s or longer shall be a long press. It acts at the 1 s mark without waiting for release.
- **BTN-3** Press edges closer together than the debounce time shall be ignored. `[DEBOUNCE_MS]`
- **BTN-4** A short press shall act as follows:
  - Off: enter the first Active Mode.
  - Idle Charging: enter the first Active Mode.
  - An Active Mode that is not the last in the Mode Cycle, inside the Selection Window (BTN-8): enter the next Active Mode.
  - The last Active Mode, or any Active Mode after the Selection Window has closed: enter Idle Charging if USB power is present, otherwise enter Deep Sleep.
- **BTN-5** A long press in any state shall enter Deep Sleep, whether or not USB power is present.
- **BTN-6** Before entering Deep Sleep the firmware shall wait for the button to be released, up to a limit, so that a still-held button does not wake the device immediately. If the limit is exceeded it proceeds anyway. `[POWEROFF_RELEASE_WAIT_MS, POWEROFF_RELEASE_POLL_MS]`
- **BTN-7** If the Deep Sleep wake configuration fails, the firmware shall stay awake in Off with the button working instead of sleeping.
- **BTN-8** Every transition from an inactive state (Deep Sleep, Off, Idle Charging) to an Active Mode shall open the Selection Window, whether caused by the button, a boot or reset, or a BLE request. Changes between Active Modes, by button or by BLE, shall neither open nor restart it. The window shall be counted in battery ticks (no dedicated timer), so its length is between (N-1) and N tick periods, about 15 to 20 s. `[MODE_SELECT_WINDOW_BATTERY_TICKS, BATTERY_SAMPLE_INTERVAL_MS]`
- **BTN-9** Closing the Selection Window shall have no visible effect. Only the effect of the next short press changes (BTN-4). USB power is evaluated at the time of that press. A long press is unaffected.
## 4. Power states

- **PWR-1** In Deep Sleep the main LED and the status LED shall be off, and BLE shall be unreachable. Only the button (or USB power appearing, see BOOT-3) wakes the device.
- **PWR-2** Off shall be entered only by a BLE request (BLE-3). It shall have no timeout and shall not enter Deep Sleep on its own.
- **PWR-3** In Off and Idle Charging the main LED shall be off.
- **PWR-4** Every automatic power-off (Smart stationary auto-off, critical battery, USB removal in Idle Charging) and the long press shall enter Deep Sleep. Only the short-press cycle (BTN-4) has a USB-dependent end.

## 5. Light output

- **LED-1** Steady shall show Base continuously. `[LIGHT_PULSE_BASE]`
- **LED-2** Flash shall show Base, with a double flash to Peak every cycle: Peak for one step, Base for one step, Peak for one step, Base for the rest of the cycle. The first double flash occurs one cycle after entering Flash. `[FLASH_CYCLE_MS, FLASH_STEP_MS, FLASH_REST_MS, LIGHT_PULSE_PEAK]`
- **LED-3** The PWM period shall be fixed. `[LIGHT_PWM_PERIOD_USEC]`
- **LED-4** Leaving Flash shall stop the flash timer. A flash step event arriving in any other state shall be ignored.

## 6. Smart

- **SMART-1** On entering Smart the main LED shall be off, and the braking and dark flags shall be cleared. Sensor sampling shall start. Sampling shall stop when Smart is left. The accelerometer and the light sensor shall be sampled on independent schedules. `[ACCEL_SAMPLING_INTERVAL_MS, LIGHT_SAMPLING_INTERVAL_MS, SENSOR_TICK_MS]`
- **SMART-2** Brightness in Smart shall be chosen by priority: braking gives Peak, otherwise dark gives Base, otherwise off.
- **SMART-3** Braking is judged on the filtered Z acceleration (SMART-8, SMART-9). The sensor is mounted with Z along the direction of travel and positive Z means decelerating and negative Z means accelerating. Braking shall start when the filtered value stays above the threshold for the start confirm time, and end when it stays at or below the same threshold for the stop confirm time plus the hold time, so the light keeps shining for the hold time after the braking phase has ended. Braking again within that time keeps the light on without a gap. The threshold is +5.0 m/s^2 (not g). `[BRAKING_ACCEL_THRESHOLD_MILLI, BRAKING_START_CONFIRM_MS, BRAKING_STOP_CONFIRM_MS, BRAKING_HOLD_MS]`
- **SMART-4** Darkness starts after consecutive samples below the dark threshold and ends after consecutive samples at or above the bright threshold. The two thresholds differ (hysteresis). `[AMBIENT_DARK_THRESHOLD_MILLILUX, AMBIENT_BRIGHT_THRESHOLD_MILLILUX, AMBIENT_SAMPLES_REQUIRED]`
- **SMART-5** After the stationary timeout of uninterrupted samples with acceleration magnitude within the tolerance band around 1g, the firmware shall enter Deep Sleep (auto-off). Any sample outside the band resets the count. A failed accelerometer read neither counts nor resets. See ADR-0002. `[STATIONARY_TIMEOUT_MS, STATIONARY_MIN_MAGNITUDE_SQ, STATIONARY_MAX_MAGNITUDE_SQ]`
- **SMART-6** Braking, darkness and auto-off events arriving in any state other than Smart shall be ignored.
- **SMART-7** If a sensor failed initialization, Smart shall continue with the remaining one. No light sensor means the LED stays off. No accelerometer means no braking and no auto-off.
- **SMART-8** The filtered Z acceleration shall be the raw Z acceleration minus a slow baseline (removes the gravity component from mounting tilt and slope). The baseline shall be seeded from the first accelerometer sample after entering Smart, follow Z with a first-order low-pass, and be frozen while braking is active or a braking start is being confirmed. `[BRAKING_BASELINE_TAU_MS]`
- **SMART-9** The baseline-corrected Z acceleration shall be averaged over a short window before it is compared to the threshold. A window of one sample disables the smoothing. `[BRAKING_SMOOTHING_MS]`
- **SMART-10** The accelerometer output data rate shall be set explicitly at start-up (not left at the driver default) and shall not be lower than the polling rate. If setting it fails, a message is logged and the driver default is used. `[ACCEL_ODR_HZ]`
- **SMART-11** Detection times (braking confirm, smoothing, baseline, stationary timeout) shall be defined in milliseconds, and the sample counts derived from them rounded up, so changing a sampling interval does not change a detection time. Ambient light detection is defined in light sensor samples. `[MS_TO_SAMPLES]`

## 7. USB power and charging

- **USB-1** USB power presence shall be polled, and an event posted only on a change. A newly plugged or unplugged cable may therefore be noticed up to one poll interval late. `[USB_POLL_INTERVAL_MS]`
- **USB-2** Plugging in USB power shall never change the state. Only the status LED changes.
- **USB-3** Removing USB power while in Idle Charging shall enter Deep Sleep. In any other state it shall only change the status LED.
- **USB-4** The firmware shall treat USB power present as Charging in every awake state. It has no knowledge of charge completion, and charging itself is done by autonomous hardware.

## 8. Battery

- **BAT-1** In every awake state the battery voltage shall be sampled periodically. `[BATTERY_SAMPLE_INTERVAL_MS]`
- **BAT-2** A failed reading shall count as 0 mV, so a battery that cannot be measured is treated as empty.
- **BAT-3** After consecutive samples below the critical voltage the firmware shall enter Deep Sleep, in every state. `[BATTERY_CRITICAL_MV, BATTERY_CONSECUTIVE]`
- **BAT-4** After consecutive samples below the low voltage the battery shall be Low. After consecutive samples at or above it, the battery shall be OK again. Entering and leaving Low use the same voltage. `[BATTERY_LOW_MV, BATTERY_CONSECUTIVE]`
- **BAT-5** Low battery shall not change the main LED. Only the status LED and the critical cut-off (BAT-3) react.
- **BAT-6** Battery readings while USB power is present are not trustworthy and no special handling exists.
- **BAT-7** Waking from Deep Sleep with a critical battery shall turn the light on until the critical cut-off triggers again, as for any other wake.

## 9. Status LED

- **STAT-1** The status LED shall show, in priority order:
  1. Low battery: slow blink. `[BLINK_INTERVAL_LOW_BATTERY_MS]`
  2. Charging: fast blink. `[BLINK_INTERVAL_CHARGING_MS]`
  3. Any Active Mode: solid on.
  4. Otherwise: off.
- **STAT-2** The status LED is visible to the rider. This is the only low-battery warning, and the main LED is unaffected until the critical cut-off.

## 10. BLE

- **BLE-1** Whenever the device is awake it shall advertise as connectable with service UUID `[BLE_SERVICE_UUID]`, restart advertising after a disconnect, and retry a failed start. Only one connection at a time is supported. A connection shall never keep the device awake. Only Off does.
- **BLE-2** Reading characteristic `[BLE_LED_STATUS_UUID]` shall return the current state as one byte (SYS-1 values).
- **BLE-3** Writing characteristic `[BLE_CONTROL_UUID]` with one byte at offset 0 shall request a state. Valid requests are Off, Steady, Flash and Smart. Idle Charging cannot be requested because only USB power sets it.
- **BLE-4** A write with a length other than one byte or a non-zero offset shall fail with an ATT error. Any other write shall return success, even if the request is ignored.
- **BLE-5** A request shall be ignored if it is not a valid request (BLE-3), equals the current state, or arrives within the stable window after the last state change (including boot). `[BLE_STABLE_WINDOW_MS]`
- **BLE-6** A request for Steady, Flash or Smart shall be applied in Idle Charging.
- **BLE-7** A request for Off shall keep the device awake and connectable.
- **BLE-8** The client reads the result back from the status characteristic. Writes are not authenticated. See ADR-0001.

## 11. Hardware assumptions

- **HW-1** The charger is autonomous hardware that charges in Deep Sleep and with a flat battery.
- **HW-2** Appearing USB power resets the MCU in hardware while in Deep Sleep.
- **HW-3** The accelerometer is mounted with Z along the direction of travel, with +Z pointing backward (against the direction of travel), so braking reads positive and accelerating reads negative.
- **HW-4** The status LED is visible to the rider.
