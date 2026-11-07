# Rear Light (ANT+)

Firmware for the bicycle rear light (BL653 / nRF52833) that a bike computer controls over **ANT+** (ANT+ Bike Lights profile). It replaces the Zephyr/BLE firmware in [`../rear_light_peripheral`](../rear_light_peripheral), keeping its behaviour (Smart, Steady, Flash, Deep Sleep, USB charging, battery cut-off) and swapping the BLE interface for ANT+. Bare metal on the ANT SoftDevice S212 7.0.1, because ANT SoftDevices are not supported by Zephyr (ADR-0005).

Tested with a Wahoo ELEMNT BOLT v2: it finds the light, shows state and battery, switches modes, and reconnects.

## Where to read what

| File | Purpose |
| --- | --- |
| [MANUAL.md](./MANUAL.md) | User manual: modes, button, charging, adding the light to a bike computer |
| [SPEC.md](./SPEC.md) | Behavioural requirements with IDs. Constants are named in `[...]` and live in `src/config.h` |
| [CONTEXT.md](./CONTEXT.md) | Glossary |
| [docs/adr/](./docs/adr) | Decisions: ANT+ replaces BLE, RC clock, unauthenticated control, reachability and silence timeout, ... |
| [VERIFY.md](./VERIFY.md) | Bench log of the prototype, what is verified on the product firmware, and the physical test checklist |
| [PORT_DECISION.md](./PORT_DECISION.md) | How the platform decision was reached (history) |

## Layout

| Path | Purpose |
| --- | --- |
| `src/sm.[ch]` | Central state machine (portable, no hardware) |
| `src/app_events.[ch]` | Event queue with coalescing of driver events (portable) |
| `src/detectors.[ch]` | Braking, ambient light and stationary detectors (portable) |
| `src/battery_logic.[ch]` | Battery thresholds and ANT+ battery mapping (portable) |
| `src/bike_light.[ch]` | ANT+ Bike Lights page logic (portable) |
| `src/config.h`, `src/board.h` | Tunable constants, pins |
| `src/main.c` | Start-up, event loop, glue between the portable core and the drivers |
| `src/ant_link.[ch]` | SoftDevice enable, channel, page transport |
| `src/light.c`, `status_led.c`, `button.c`, `battery.c`, `power.c`, `soft_timer.c`, `twim.c`, `sensors.c`, `platform.c` | Drivers (registers, no HAL) |
| `src/startup.c`, `linker/rear_light_ant.ld` | Vector table, reset handler, memory map (application at 0x12000, RAM above the SoftDevice) |
| `test/` | Host tests of the portable core |
| `tools/` | Debug counters and bench helpers over SWD |

## Build

Needs the GNU Arm toolchain of the nRF Connect SDK installation, the S212 SoftDevice archive and (for flashing) `nrfjprog` plus a J-Link. No nRF5 SDK.

```sh
make            # build/rear_light_ant.hex (unzips the SoftDevice from ../ant_source on first run)
make test       # host tests (gcc, ASan/UBSan)
```

Paths default to this machine's NCS install; override with `make NCS=... TOOLCHAIN=...`. `make V=1` shows the full commands. Options:

| Option | Meaning |
| --- | --- |
| `LFCLK=rc` (default), `synth`, `xtal` | Low-frequency clock: internal RC (the board has no crystal, ADR-0006), derived from the 32 MHz crystal, or a 32.768 kHz crystal |
| `BENCH=1` | Bench image: fixed battery value and event injection over SWD. **Never put this image in a light that is used on a bike**: it has no battery cut-off |
| `FORCE_SD_FAILURE=1` | Test image that pretends the SoftDevice cannot start |

The ANT+ profile documents and SoftDevice files are covered by the ANT+ Adopter Agreement and the SoftDevice licence and must not be redistributed: `../ant_source/`, `build/` and `../ant_docs/` are in `.gitignore`.

## Keys

The firmware needs two keys. They are **not in this repository** and must not be committed or published. Each developer provides them locally:

1. `cp keys.example.mk keys.local.mk`
2. Fill in both values in `keys.local.mk`. The file is git-ignored.

| Key | Where to get it | Notes |
| --- | --- | --- |
| `ANT_PLUS_NETWORK_KEY` (16 hex digits) | Garmin, [ANT+ network keys](https://developer.garmin.com/ant-program/ant-ant-plus/network-keys/) | Only for ANT+ Adopters. Garmin says not to publish it and not to use it for non-ANT+ devices |
| `ANT_LICENSE_KEY` (`xxxx-…-xxxx`) | The S212 SoftDevice evaluation key from Nordic/ANT (`nrf_sdm.h` in the SDK), or a commercial key from [Garmin licensing](https://developer.garmin.com/ant-program/licensing/) | The evaluation key is **non-commercial use only** |

`make` (and `make flash`) stops with a message if a key is missing or malformed. `make test` needs no keys. The Makefile writes `build/ant_keys.h` from `ANT_PLUS_NETWORK_KEY`; `ANT_LICENSE_KEY` is passed to the compiler as a macro.

To pass a key on the command line instead of the file: `make ANT_LICENSE_KEY=...`.

## Flash

```sh
make flash       # first time: chip erase, SoftDevice + application, reset
make flash-app   # application only
```

`make flash` erases everything on the chip (including the Zephyr firmware). The application image also writes UICR `NFCPINS` so P0.09 (main LED) is a GPIO. To return to the Zephyr firmware: `nrfjprog --family NRF52 --recover`, then build and flash `../rear_light_peripheral` as before.

## Bench tools

All read the target without stopping it. Without a battery on the board use a `BENCH=1` image.

| Command | Purpose |
| --- | --- |
| `make stats` | Counters: state, ANT channel and events, last received pages, sensor values, resets, SoftDevice faults |
| `make rate RATE_S=30` | ANT TX event rate and LF clock check (the host clock in WSL runs slow, see VERIFY.md) |
| `make watch WATCH_S=300` | Log changes over time (reconnect tests) |
| `make inject EVT=BUTTON_LONG` | `BENCH=1` only: post an event (`BUTTON_SHORT`, `BUTTON_LONG`, `BUTTON_WAKE`, `USB_CONNECTED`, `USB_DISCONNECTED`, `REMOTE_SET_STATE ARG=n`, ...) |

The J-Link link from WSL can drop (usbipd); `make stats` then fails with "could not read target memory" instead of showing stale data.

## Status

Host tests cover the logic; the board runs the full state machine, the sensors and the ANT+ link on the bench. The physical checklist in [VERIFY.md](./VERIFY.md) (real button, USB-C, Smart detectors with real stimuli, battery thresholds, current, range, 8 h run, Wahoo with this firmware) is still open.
