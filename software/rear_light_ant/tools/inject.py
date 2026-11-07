#!/usr/bin/env python3
"""Post an application event on a bench build (make BENCH=1) over SWD.

usage: inject.py <elf> <nm> <event name> [arg]

The event is picked up within a second (the USB poll timer wakes the main
loop even in Deep Sleep). Event names are those of app_events.h without the
EVT_ prefix, e.g. BUTTON_SHORT, BUTTON_LONG, BUTTON_WAKE, USB_CONNECTED,
REMOTE_SET_STATE 1.
"""
import subprocess
import sys
import tempfile

import stats

EVENTS = [
    "BUTTON_SHORT", "BUTTON_LONG", "REMOTE_SET_STATE", "FLASH_STEP", "BATTERY_TICK",
    "USB_CONNECTED", "USB_DISCONNECTED", "BRAKE_START", "BRAKE_STOP", "AMBIENT_DARK",
    "AMBIENT_BRIGHT", "STATIONARY_TIMEOUT", "BUTTON_WAKE", "MOTION_DETECTED",
]


def main():
    if len(sys.argv) not in (4, 5):
        sys.exit(__doc__)
    name = sys.argv[3].upper()
    if name not in EVENTS:
        sys.exit("unknown event, use one of: " + ", ".join(EVENTS))
    arg = int(sys.argv[4]) if len(sys.argv) == 5 else 0
    address = stats.symbol_address(sys.argv[1], sys.argv[2])
    inject_address = address + stats.struct.calcsize(stats.FORMAT) - 4
    value = (1 << 16) | (arg << 8) | EVENTS.index(name)
    with tempfile.NamedTemporaryFile("w", suffix=".jlink") as f:
        f.write(f"connect\nw4 0x{inject_address:08X} 0x{value:08X}\nexit\n")
        f.flush()
        out = subprocess.run(
            ["JLinkExe", "-device", "NRF52833_XXAA", "-if", "SWD", "-speed", "4000",
             "-autoconnect", "1", "-NoGui", "1", "-CommandFile", f.name],
            check=True, capture_output=True, text=True,
        ).stdout
    if "FAILED" in out or "Cortex-M4" not in out:
        sys.exit("J-Link could not connect to the target:\n" + out)


if __name__ == "__main__":
    main()
