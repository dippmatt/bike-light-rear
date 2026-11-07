#!/usr/bin/env python3
"""Log changes of the debug counters over time (J-Link, CPU keeps running).

Prints one line whenever the mode, light index, received messages, event
counters other than TX, or the uptime (a reset) change. Used for the
reconnect tests: start it, switch the controller or the light off and on, and
read the timeline afterwards.

usage: watch.py <elf> <nm> [seconds] [interval]
"""
import struct
import sys
import time

import stats




def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    address = stats.symbol_address(sys.argv[1], sys.argv[2])
    seconds = float(sys.argv[3]) if len(sys.argv) > 3 else 300.0
    interval = float(sys.argv[4]) if len(sys.argv) > 4 else 1.0
    start = time.monotonic()
    last_key = None
    last_tx = None
    while time.monotonic() - start < seconds:
        v = stats.sample(address)
        s = dict(zip(stats.FIELDS, v[:13]))
        mode, light_index = v[15], v[16]
        events = v[17:17 + 0x42]
        rx_next = v[17 + 0x42 + 2]
        other_events = {stats.EVENT_NAMES.get(i, hex(i)): c for i, c in enumerate(events)
                        if c and i not in (3, 0x40)}
        key = (s["uptime_s"] // 1000, mode, light_index, s["rx_messages"], s["mode_changes"],
               tuple(sorted(other_events.items())), s["magic"])
        reset = last_tx is not None and s["tx_events"] < last_tx
        last_tx = s["tx_events"]
        if key != last_key or reset:
            t = time.monotonic() - start
            print(f"{t:6.1f}s up={s['uptime_s']:4d} tx={s['tx_events']:5d} rx={s['rx_messages']:3d} "
                  f"cmds={s['rx_commands']:3d} mode={stats.MODES.get(mode, mode):6} "
                  f"index={light_index} events={other_events}{'  RESET' if reset else ''}",
                  flush=True)
            last_key = key
        time.sleep(interval)


if __name__ == "__main__":
    main()
