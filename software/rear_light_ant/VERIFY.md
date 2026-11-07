# Bench verification

The prototype has only been compiled and unit-tested on the host. This
document is the procedure that turns it into evidence for the decision in
[PORT_DECISION.md](PORT_DECISION.md). Fill in the **Result** columns while you
test; the decision gate reads them.

## Equipment

- Rear light board (BL653) with SWD access and a J-Link, `nrfjprog` installed.
- The real ANT+ bike computer that the light is meant for (**required**).
- Optional but useful: an ANT USB stick plus SimulANT+ (Garmin tool in
  `software/ant_source/Software_Tools`) or any ANT+ capable phone/PC app. It
  shows raw pages and lets you send page 34 manually, so a failure on the bike
  computer can be told apart from a failure of the light.
- Optional for T6: a current meter that resolves 10 µA to 5 mA (Nordic PPK2 or
  a µCurrent), in series with the battery.

## Setup

```sh
cd software/rear_light_ant
make test                      # host tests must pass
make flash                     # S212 + application
make stats                     # sanity check: magic OK, uptime counting
```

Expected after boot: main light steady, status LED pulses once. If the status
LED blinks repeatedly instead, the SoftDevice did not start; the number of
blinks is the error code (see README), and `make stats` shows `last_error` and
`last_error_site`.

## Tests

| # | Test | How | Expected | Result |
| --- | --- | --- | --- | --- |
| T1 | SoftDevice starts on the RC clock | `make flash`, `make stats` | `last_error` 0, `uptime_s` increases, `tx_events` increases | **Pass** (J-Link, nRF52833 rev 1): `last_error` 0, `sd_fault_*` 0 |
| T2 | Event rate | `make rate RATE_S=60` | TX rate about 8.02 Hz and LF clock error near 0 % | Measured 8.67 Hz against the WSL clock, which turned out to run about 9 % slow (see "Clock question" at the end); not a firmware error |
| T3 | Found by controller | Bike computer: add sensor / light, search. With SimulANT+: search for device type 35 | A light appears; the device number is the low 16 bits of the chip's `DEVICEID[0]` (`nrfjprog --memrd 0x10000060 --n 4`) | |
| T4 | Light state shown | On the controller, check light state and battery | Mode shows steady; battery roughly matches the cell | |
| T5 | Commands (**pass for the BOLT toggle**, see bench log) | From the controller switch Off / Steady / Flash / Auto and back, 10 times each | Main light follows within about 1 s; status LED pulses; `rx_commands` increases; `last_rx` shows page 34; the controller shows the new state afterwards (page 1 echo) | |
| T6 | Controller UI options | Note which modes the controller offers (page 2 advertises Off, Steady, Slow flash, Auto) | List the modes the UI shows and what each does | |
| T7 | Page requests | Controller or SimulANT+ requests pages 2, 80, 81, 19 | `rx_requests` increases; requested page appears in `last_tx` within a few events | |
| T8 | Button | Press the board button | Steady → Flash → Smart → Off; the controller shows the change | |
| T9 | Range and dropouts | Walk away to 5 m, 10 m, 20 m with the controller; also mount on a bike at the rear and ride | Link holds; note the distance where commands stop | |
| T10 | RC-clock stability | Leave running 8 h (controller connected and idle). Run `make rate RATE_S=60` and `make stats` at start and end. Repeat once warm (hair-dryer on the board) and once cold (fridge, in a bag) | `tx rate` stays within 8.02 Hz ± 0.1; `sd_fault_id` 0; no resets (`uptime_s` keeps counting); commands still work at the end | |
| T11 | Reconnect (**pass for controller off/on and range, light power cycle not tested**, see bench log) | Switch controller off and on, leave range and return, power-cycle the light | Controller finds the light again without re-pairing (ANT+ has no pairing; the device number must be unchanged) | |
| T12 | Average current | PPK2 / µCurrent for 10 min, light off, controller present and absent | Record µA. Compare with the same measurement of the Zephyr BLE firmware (advertising / connected, light off); no figure for it is recorded in the repo | |

## Results to record

| Item | Value |
| --- | --- |
| Controller make/model/firmware | |
| LF clock | RC (default) / crystal |
| TX rate and LF clock error after 8 h, cold / warm | |
| Number of dropouts or resets in T10 | |
| Maximum distance with working commands | |
| Average current, light off, with controller [µA] | |
| Average current, light off, no controller [µA] | |
| Modes the controller can select | |
| Anything in the pages the controller rejected or ignored (`VERIFY` markers) | |

## If the link is unstable on the RC clock

Run T10 again with a crystal. Solder a 32.768 kHz crystal (BL653 datasheet:
XL1/XL2 pads) with load capacitors per the datasheet and build with
`make LFCLK=xtal`. If the crystal fixes it, the RC clock is the cause and
the next board revision needs the crystal (decision in PORT_DECISION.md).

## If the controller does not show the light or ignores commands

Check in this order: T3 with SimulANT+ (is the light visible at all), compare
`last_tx` against the profile page layout, check the two `VERIFY` markers in
`src/bike_light.c`, then check whether the controller expects the light to
appear as an **unconnected** light with index 0 (what the prototype does) or
as a member of a light network. Record the answer in the table above; it
changes the profile logic in the port.

## Bench log

### 16 Oct 2026, J-Link on the real board (no bike computer yet)

- Target: nRF52833 rev 1, `make flash` ok (previous firmware backed up to
  `build/zephyr_backup.hex`, restore with `nrfjprog --program` after
  `--recover`).
- SoftDevice enables on the RC clock, channel opens, 8 TX events per second,
  `last_error` 0, no SoftDevice fault.
- `make rate`: TX rate 8.67-8.70 Hz instead of 8.02 Hz, and RTC1 (1024 Hz from
  the LF clock) runs 8.2 % fast against the host clock. The same +8.2 % shows
  with `LFCLK=synth` (`CLOCK->LFCLKSTAT` confirms SRC = synth). Because RC
  (calibrated against HFCLK) and synth both follow the high-frequency clock,
  either HFCLK itself is off or the host reference is. The host reference
  was the culprit: see "Clock question" at the end of this file.

### 16 Oct 2026, Wahoo ELEMNT BOLT v2

- The BOLT finds the light, shows its state (Auto / Aus / 20 % / Langsamer
  Flash, so its page 1 decoding matches) and follows button changes on the
  light. **T3, T4 and T8 pass.**
- First attempt, commands from the BOLT ("Umschalten") did nothing. The log of
  received messages (`make stats`) showed why: the BOLT sends the connect
  command (page 33, `21 01 01 00 00 00 00 00`) and then page 34 to **light
  index 1**; the light ignored page 33 and only accepted index 0. Fixed by
  implementing the connected main-light role (README).
- Second finding: the BOLT then commanded mode 63 (`22 01 20 nn 00 00 fe ff`)
  after slow flash and auto. Page 2 advertised 4 modes but only 3 standard
  modes in the bit field, so the controller assumed one custom mode (63).
  Fixed by counting 3 modes (profile 5.3, custom modes count down from 63).
- Re-test with the fixed mode count: the BOLT's toggle now cycles
  Auto (`26`) → Off (beam bits `01`) → 20 % (`16`) → slow flash (`1a`) → Auto
  and the light follows every step (`mode_changes` and `last_rx` agree).
  **T5 passes for the toggle button** (about 10 presses). Still open: the BOLT's
  other UI paths, requests (T7 `rx_requests` stayed 0), and T6 whether the BOLT
  UI lists the modes by name.
- Tooling lesson: `nrfjprog --memrd` followed by `--run` restarts the chip from
  reset, so reading counters that way wiped them and dropped the ANT link. The
  tools now read RAM through a J-Link session that does not halt the CPU.

### T11 reconnect, Wahoo ELEMNT BOLT v2 (7 min timeline, `make watch`)

- Controller switched off and on, and out of range and back: the light stayed
  connected (index 1), the BOLT showed it again without re-adding it and
  UMSCHALTEN worked afterwards; no reset, no ANT channel event other than TX/RX
  in the log. Result as expected. **Pass.**
- While connected the BOLT sends `12 ff ff ff ff 00 00 00` (page 18 shape,
  device number 0) about every 24 s and the light counts it as a received
  message. This is the controller's keep-alive ("ping") of profile 5.5. The
  profile's 2-minute timeout (return to unconnected, beam off) can be
  implemented in the port on top of it; not done in the prototype.
- Not tested: powering the light itself off and on while the BOLT stays on
  (the board was on the J-Link, not the battery).

### Clock question (decision of 16 Oct 2026: closed, no further measurement)

The +8.2 % "LF clock error" was measured against the host clock in WSL. A
check of that reference against NTP (`pool.ntp.org`, 60 s sleep) showed NTP
advancing 65.5 s while the WSL monotonic clock advanced 60.0 s, i.e. the WSL
clock runs about 9 % slow. The board's RTC/host ratio of 1.082 is therefore
consistent with a correct LF clock. The board was not re-measured against NTP;
the project decision is to accept the RC clock because the link to the Wahoo
works. `LFCLK=xtal` stays a build option. Do not use the WSL clock as a timing
reference for `make rate`.

## Product firmware: verification status (16 Oct 2026)

Everything above is the bench prototype. The product firmware (SPEC.md) is
built from it and was checked in three ways: host tests of the portable core
(`make test`, names carry the requirement IDs), the real board with events
injected over SWD (`make BENCH=1 flash-app`, `make inject EVT=...`), and
physical tests that only a person at the board can do (listed last).

### Covered by host tests (`make test`)

| Requirements | Test |
| --- | --- |
| SYS-2, SYS-3, SYS-4 | `test_SYS2_mode_cycle_order`, `test_SYS3_SYS4_queue_order_and_drop`, `test_SYS4_driver_events_are_coalesced` |
| BOOT-1, BOOT-2 | `test_BOOT1_BOOT2_boot_policy` |
| BOOT-3, BOOT-4 | `test_BOOT3_*`, `test_BOOT4_*` (wake, held button, re-enable failure resets) |
| BTN-4 .. BTN-9 | `test_BTN4_*`, `test_BTN5_*`, `test_BTN6_*`, `test_BTN7_*`, `test_BTN8_*`, `test_BTN8_BTN9_selection_window` |
| PWR-1 .. PWR-4 | `test_PWR1_*`, `test_PWR2_PWR3_*`, `test_PWR4_all_power_off_paths` |
| LED-1, LED-2, LED-4 | `test_LED1_LED2_steady_and_flash`, `test_LED4_*` |
| SMART-1 .. SMART-6, SMART-8, SMART-9, SMART-11 .. SMART-13 | `test_SMART*` (detectors on synthetic samples, state machine priority rules) |
| SMART-5, SMART-16 .. SMART-19 (Parked) | `test_SMART5_SMART16_*`, `test_SMART16_*`, `test_SMART17_*`, `test_SMART18_*`, `test_SMART19_*`, `test_SMART_auto_off_variant_*` (state machine only; the LIS3DH motion detector needs the board, P9a/P9b) |
| USB-2 .. USB-4 | `test_USB2_USB3_USB4_usb_edges` |
| BAT-2 .. BAT-5, BAT-7 | `test_BAT2_BAT3_BAT4_battery_logic`, `test_BAT5_*`, `test_BAT7_*` |
| STAT-1 | `test_STAT1_status_led_priority` |
| ANT-3 .. ANT-12 | `test_bike_light.c` (pages, mode map, connect/disconnect, commands, silence) and `test_ANT7_*`, `test_ANT8_*`, `test_ANT9_*`, `test_ANT12_*`, `test_PWR2_PWR3_*` (Off stays awake) |

### Checked on the real board with injected events

Powered from the J-Link with `make BENCH=1` (fixed battery value, event
injection). Results of 16 Oct 2026:

| Requirement | Result |
| --- | --- |
| SYS-3, SYS-4 | Four-minute soak: no dropped event, no fault, no reset |
| BOOT-1 data path | After a power cycle `reset reason` reads 0 (power-on); after `nrfjprog --reset` it reads 0x4. USB with a power-on reset still to be tested physically |
| BTN-4, BTN-5 (logic), PWR-1, ANT-2 | Short and long press events step through the cycle, enter Deep Sleep, and the ANT channel closes (`tx_events` stops) and opens again on wake |
| BOOT-3, USB-3 | USB connected wakes into Idle Charging, USB removed in Idle Charging enters Deep Sleep |
| SMART-7, SMART-10, SMART-14 | LIS3DH and OPT3001 are found and configured; readings are plausible (about 1 g on the gravity axis, 250 lux in the room). Sensors power down in Deep Sleep and come back after wake |
| STAT-1 (heartbeat) | Status LED pin sampled over SWD every 100 ms: Smart solid on; Off set by the controller one 100 ms pulse every 2 s; Deep Sleep dark; solid again after wake |
| SYS-5 | `make BENCH=1 FORCE_SD_FAILURE=1`: SoftDevice enable fails (error 0x3), the light and button logic, timers and Deep Sleep still work without it |
| ANT-1, ANT-4, ANT-5, ANT-13, ANT-15 | Channel opens, pages are sent at the expected rate, no SoftDevice fault, counters readable with `make stats` |

### Smart parked, sample board (7 Nov 2026, `make BENCH=1`, J-Link)

Status LED pin (P0.11) sampled over SWD about every 1 ms, timed by the board's RTC1 counter.

| Requirement | Result |
| --- | --- |
| SMART-16 | `make inject EVT=STATIONARY_TIMEOUT`: `make stats` shows `Smart (parked)`, the ANT+ mode stays Smart |
| SMART-17 | Board shaken by hand: back to full Smart within the next poll |
| SMART-17 (no false wake) | About 2.5 minutes untouched on the table: stayed parked (300 mg threshold) |
| STAT-1 (parked pulse) | 30 ms every 5000 ms |
| STAT-1 (heartbeat) | 100 ms every 2000 ms |

Bug found and fixed: before the fix both pulses measured about 124 ms (parked period 5110 ms, heartbeat period 2118 ms). The main loop sleeps inside an SoftDevice critical region, which masks application interrupts, and `SEVONPEND` was only set without the SoftDevice. Timer events therefore waited for the next ANT radio event (about 125 ms), which also stretched Flash steps and button sampling. `SEVONPEND` is now set in both cases (`src/main.c`). The earlier heartbeat result above was sampled every 100 ms and could not show this.

### Physical tests still to do (checklist)

Fill the Result column; none of these can be done through the debugger.

| # | Test | How | Expected | Result |
| --- | --- | --- | --- | --- |
| P1 | Real button, short press | Press and release quickly, 10 times | Each press steps through Smart, Steady, Flash, then sleeps (inside the first 15 s); `make stats` `mode_changes` agrees | |
| P2 | Real button, long press and wake | Hold 1 s; wait; press once | Light goes off and the ANT channel closes (`ant channel closed`); the next press wakes into Smart without stepping on | |
| P3 | Button bounce and double press | Press twice within 0.2 s | The second press is ignored (BTN-3) | |
| P4 | Flash and steady look | Watch the main LED in Steady and Flash | Same look as the Zephyr firmware (double flash every 1.5 s) | |
| P5 | USB-C plug with the light off, on and in Deep Sleep | Plug and unplug | Fast status blink while charging; wake into Idle Charging from Deep Sleep within about 1 s; unplug in Idle Charging sleeps | |
| P6 | Power-on reset with USB present | Connect the battery with USB-C plugged in | Boots into Idle Charging (BOOT-1); `reset reason 0` | |
| P7 | Smart: ambient light | Cover the sensor, then uncover; watch `make stats` | `ambient` drops below 30 lux, the LED goes to Base within a few seconds, the blanked measurement shows as one brief dark moment every 30 s (`blanked measurements` counts up); uncover and it turns off again | |
| P8 | Smart: braking | Move the light (jerk it along its Z axis, see HW-3) with the LED dark | LED goes to Peak while decelerating and holds 0.5 s | |
| P9 | Smart: parking | Leave the light still in Smart for 2.5 minutes (or `make inject EVT=STATIONARY_TIMEOUT` on a BENCH build) | Main LED off, status LED one 30 ms flash every 5 s, `make stats` shows `Smart (parked)`, the bike computer still shows Auto | |
| P9a | Smart: auto-on | While parked, tap the light or lift the bike | Within about 1 s the status LED is solid again and `(parked)` is gone; braking boost works | |
| P9b | Smart: no false wake | Leave it parked on a table for 30 minutes, then on the bike in the wind | Stays parked; if it wakes by itself, raise `MOTION_WAKE_THRESHOLD_MG` | |
| P9c | Parked current | Meter in series with the battery: Smart with LED off, then Parked | Record both; Parked should be clearly lower (no light sensor, no 20 Hz polling) | |
| P10 | Low and critical battery | Supply from a bench supply, lower slowly (no `BENCH=1`!) | Status LED blinks slowly below 3.4 V, the light powers off below 3.0 V | |
| P11 | Wahoo with the product firmware | Press UMSCHALTEN several times, switch Off/On, wait out the Stable Window | Same behaviour as the prototype: commands follow, `light index 1`; a request right after a button press is ignored for 3 s | |
| P12 | Silence timeout | Connect with the Wahoo, then switch the Wahoo off for 3 minutes | `make stats`: light index 0, `silence timeouts 1`, the main LED unchanged | |
| P13 | Current | Meter in series with the battery, light off (Deep Sleep), Off with the controller present, Steady | Record; compare with the Zephyr firmware | |
| P14 | Range | Walk away with the Wahoo | Note the distance where commands stop | |
| P15 | 8 h stability | Leave running with the Wahoo nearby (`make watch WATCH_S=28800`) | No reset, no dropped events, commands work at the end | |
