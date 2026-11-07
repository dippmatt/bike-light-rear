# Decision gate: how to bring ANT+ into the product firmware

> **Decided 16 Oct 2026:** path A (evolve this bare-metal code base), S212 (ANT only, no BLE), RC clock kept, Deep Sleep unreachable and no beam-off timeout. See ADR-0005, ADR-0006 and ADR-0007 in `docs/adr`. This file is kept as the record of how the decision was prepared; the port plan below was carried out in the product firmware (SPEC.md, `src/`).

Input: the results table in [VERIFY.md](VERIFY.md). Nothing below has been
decided by measurement yet; the first section lists what is already known
without hardware, the second turns the possible outcomes into a decision.

## What is known without hardware

1. **Zephyr / NCS cannot host the ANT stack.** The ANT SoftDevices are only
   delivered as nRF5-SDK-era binaries (S212, S312, S340) with SVC calls and
   their own interrupt rules. NCS has no support for them. Any ANT+ light
   therefore leaves the current Zephyr firmware architecture; it is not a
   `ble.c` swap.
2. **Two SoftDevice options fit the nRF52833** (from the 7.0.1 release notes in
   `software/ant_source`):

   | SoftDevice | Contents | Flash / RAM | nRF52833 |
   | --- | --- | --- | --- |
   | S212 7.0.1 | ANT only | 72 kB / 2.94 kB | yes (used by the prototype) |
   | S340 7.0.1 | ANT + BLE concurrently (S140 based) | 200 kB / 8 kB minimum | yes (also nRF52820, nRF52840) |
   | S312 7.0.1 | ANT + BLE peripheral (S112 based) | 144 kB / 4.9 kB | **not listed** for nRF52833 |

   The BL653 has 512 kB flash and 128 kB RAM, so either fits. S340 is the only
   way to keep the existing BLE control (SPEC BLE-1..8, ADR-0001) next to ANT+.
3. **The internal RC low-frequency clock is not supported for ANT.** The
   release notes of S212, S312 and S340 all say the RC source is "not tested or
   intended for use with the ANT stack". The board has no 32.768 kHz crystal
   (XL1/XL2 unconnected). This is the main risk and the reason for the bench
   prototype. A crystal means a board revision (BL653 datasheet shows the pads,
   no module change).
4. **Licence.** The ANT SoftDevice needs a licence key; the evaluation key is for
   non-commercial use only. The ANT+ network key requires the ANT+ Adopter
   Agreement and certification for use in a shipped product. Neither key is
   committed: both live in the developer's git-ignored `keys.local.mk` (README.md,
   "Keys"). This applies to every path below.
5. **ANT+ has no authentication.** Like ADR-0001 for BLE, any controller in range
   can command the light. That needs its own ADR in the port.
6. **Deep Sleep means unreachable.** In the current firmware Deep Sleep stops
   BLE (PWR-1). With ANT+ the channel would be closed too, so a bike computer
   can only switch the light on while it is awake (Off keeps it awake and
   reachable, BLE-7). Whether this is acceptable depends on test T12 (current
   with channel open) and how the light is used. It is a product decision, not
   a technical one.

7. **Bike computers form a network even with one light (bench result).** The
   Wahoo ELEMNT BOLT v2 sends the connect command (page 33) to make the light
   the "main light" (light index 1) before it sends any mode command. The
   port must keep that connected state (the prototype implements it without
   the shared channel). A front light would have to be a secondary light on the
   shared channel (profile 6.4/6.5), which is more ANT channel work than the
   rear light alone and favours keeping both lights in the same firmware
   family.
8. **Mode list is what the controller offers.** The BOLT builds its mode
   toggle from page 2 (here: steady 0-20 %, slow flash, auto). Off is not in
   the list; it comes from the controller's own Off action or the button on the
   light. The mode table in `bike_light.c` is the single place to change it.

9. **Keep-alive observed.** The BOLT sends a ping every ~24 s (VERIFY.md,
   T11). A timeout of two minutes without any controller message (profile
   5.5) is therefore realistic for the port, but its "beam off" side effect
   needs a product decision first.

## Decision table

Evaluate top to bottom; the first matching row wins.

| Result of the bench | Meaning | Decision |
| --- | --- | --- |
| T1 or T10 fails on RC, passes with crystal | Link needs the crystal | **Continue with ANT+, require a crystal in the next board revision** (PCB change), then follow path A or B below. Until then ANT+ is not releasable on the current board. |
| T1/T10 fail on RC and with crystal | Prototype bug or hardware problem | Debug first (`last_error`, `sd_fault_*`, SimulANT+). Do not start the port. |
| T3/T5/T7 fail on the real controller but work with SimulANT+ | Controller expects something else (light network, different page layout) | Fix the profile logic in `bike_light.c` first (host-testable), re-run the bench. No impact on the platform decision. |
| T3-T11 pass on RC, T12 within budget | ANT+ viable on current hardware | Choose a port path below. |
| T3-T11 pass on RC, T12 over budget | Works but battery life unacceptable | Treat Deep Sleep as ANT-unreachable (point 6), or add a crystal; re-measure. |

## Port paths (when ANT+ is viable)

### A. Evolve this bare-metal prototype into the product (recommended starting point)

Keep `src/bike_light.c`, `ant_link.c` and the event structure; port the
remaining Zephyr modules onto it one at a time.

- Pro: already builds, tiny (about 4.3 kB flash used of 448 kB available for
  the app), no nRF5 SDK or RTOS licence surface, the profile logic is already
  hardware-free and host-tested, the architecture mirrors the Zephyr firmware
  (IRQs post events, one loop owns state).
- Con: every driver is written against the registers (PWM, SAADC, TWIM for the
  sensors, GPIOTE, USB detect, UICR). No RTOS or logging/RTT; debug is via SWD
  and `make stats`. The team must maintain these drivers.

### B. Port to the nRF5 SDK 17.1.0 (S212 or S340 plus `ant_*` libraries)

- Pro: Nordic's drivers and `app_timer`, `nrf_pwr_mgmt`, `app_button`,
  `nrf_drv_twi` replace the hand-written ones; most ANT+ examples are written
  for it; `ant_bsp`/`ant_state_indicator` available.
- Con: SDK is end-of-life (17.1.0 is the last), build system is Makefile/SES
  based, a second dependency set next to NCS; the SDK is only needed because
  of its libraries, the prototype shows the SoftDevice itself does not need it.
- Choose B over A only if the team prefers vendor drivers to maintaining
  register code. The bench result does not change this.

### C. S340 (ANT + BLE) to keep BLE control

Needed only if BLE control must remain. With S340 the application is still an
nRF5-SDK-style app (path A or B) and BLE must be reimplemented against the
S140 API (GATT service, advertising), replacing `ble.c`. Larger SoftDevice
(200 kB, 8 kB RAM) and a more complex timing situation (ANT and BLE share the
radio) with the RC clock problem unchanged. Not recommended unless a phone
configuration path is a requirement.

### D. Not recommended

- Staying on Zephyr and adding a bridge MCU/transceiver for ANT+: two chips
  and a UART protocol for one light.
- Changing MCU to something Zephyr supports for ANT: no Zephyr-supported ANT
  radio exists.

## Port plan (path A, used as the working default until the bench decides)

Each step is a separate, testable change; the profile logic never depends on
the hardware step. Requirement IDs refer to `rear_light_peripheral/SPEC.md`.

| Step | Work | Verify |
| --- | --- | --- |
| 1 | Extract the state machine from `rear_light_peripheral/src/state_machine.c` (SYS, BTN, BOOT, PWR requirements) into a hardware-free module with a host test suite, replacing its Zephyr events with the prototype's event flags | Host tests reproduce the SPEC behaviour for button/BLE-5 stable window |
| 2 | Replace `BLE-1..8` with ANT+ requirements: channel open whenever awake; Off keeps it reachable; commands validated as in `handle_settings`; stable window (old BLE-5) applied in `on_remote_mode_request`; Deep Sleep behaviour from point 6 | SPEC reviewed and agreed before code |
| 3 | Drivers: TWIM for the sensors (`sensors.c`, Smart mode), power/USB detect and charge state, Deep Sleep (System ON idle through `sd_app_evt_wait`, ADR-0004 still applies) | Bench: Smart mode, USB wake, T12 current |
| 4 | Persist nothing new; remove `ble.c`, Zephyr build files and BLE Kconfig from the product tree once the ANT build passes all prior bench tests | Full regression against SPEC |
| 5 | Product concerns: licence key from ANT Wireless/Garmin, ANT+ Adopter certification, own manufacturer ID (the prototype uses 0x00FF), version numbers, DFU/bootloader plan (MBR + S212 layout), watchdog | Release checklist |

## Documents to update once the path is chosen

Do this only after the decision, not before (the current firmware and docs
describe a working BLE product).

| File | Change |
| --- | --- |
| `rear_light_peripheral/SPEC.md` | Replace section BLE (BLE-1..8) by the ANT+ requirements from step 2 |
| `rear_light_peripheral/CONTEXT.md` | Terms: ANT+ Bike Light, Light Controller, Light Mode vs ANT+ mode (mapping table); drop BLE terms that no longer exist |
| `rear_light_peripheral/MANUAL.md` | How to pair / find the light on the bike computer; supported modes |
| `rear_light_peripheral/README.md`, root `README.md` | Build and flash instructions for the new toolchain; remove Zephyr/NCS build text for the main product |
| `rear_light_peripheral/docs/adr/` | New ADRs: (5) ANT+ replaces BLE and why Zephyr is left; (6) LF clock source (RC or crystal, from T10); (7) no authentication for ANT+ control (supersedes ADR-0001 for the transport); (8) Deep Sleep reachability. Mark ADR-0001 and ADR-0004 as superseded or amended as appropriate |
| `hardware/` | Crystal footprint and load capacitors on XL1/XL2 if T10 requires it |

## Recommendation

Run the bench first; do not start steps 1-5 before T3 to T11 have been filled
in on the real bike computer. If they pass, path A is the lowest-risk way
forward; if the RC clock is the problem, the schematic change is the
critical path and starts before any firmware port work.
