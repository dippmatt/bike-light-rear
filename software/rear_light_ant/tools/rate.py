#!/usr/bin/env python3
"""Measure the ANT TX event rate and the low-frequency clock error on the target.

Uses one J-Link session that reads RTC1 COUNTER (1024 Hz, clocked by the LF
clock) and the TX event counter without halting the CPU, with a J-Link
"sleep" in between as the host time reference. The ANT channel period is
counted in LF clock ticks, so a wrong LF clock shows up both as a wrong RTC
time and as a wrong event rate.

usage: rate.py <elf> <nm> [seconds]
"""
import re
import subprocess
import sys
import tempfile

RTC1_COUNTER = 0x40011504
RTC_HZ = 1024.0
RTC_WRAP = 1 << 24
NOMINAL_RATE_HZ = 32768.0 / 4084.0
TX_EVENTS_OFFSET = 8  # struct proto_stats, third word


def symbol_address(elf, nm):
    out = subprocess.run([nm, elf], check=True, capture_output=True, text=True).stdout
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[2] == "proto_stats":
            return int(parts[0], 16)
    sys.exit("proto_stats not found in " + elf)


def main():
    if len(sys.argv) not in (3, 4):
        sys.exit(__doc__)
    seconds = float(sys.argv[3]) if len(sys.argv) == 4 else 30.0
    tx_addr = symbol_address(sys.argv[1], sys.argv[2]) + TX_EVENTS_OFFSET

    script = (
        "connect\n"
        f"mem32 0x{RTC1_COUNTER:08X} 1\nmem32 0x{tx_addr:08X} 1\n"
        f"sleep {int(seconds * 1000)}\n"
        f"mem32 0x{RTC1_COUNTER:08X} 1\nmem32 0x{tx_addr:08X} 1\n"
        "g\nexit\n"
    )
    with tempfile.NamedTemporaryFile("w", suffix=".jlink") as f:
        f.write(script)
        f.flush()
        out = subprocess.run(
            ["JLinkExe", "-device", "NRF52833_XXAA", "-if", "SWD", "-speed", "4000",
             "-autoconnect", "1", "-NoGui", "1", "-CommandFile", f.name],
            check=True, capture_output=True, text=True,
        ).stdout

    values = [int(m, 16) for m in re.findall(r"^[0-9A-Fa-f]{8} = ([0-9A-Fa-f]{8})", out, re.M)]
    if len(values) != 4:
        sys.exit("unexpected J-Link output:\n" + out)
    rtc0, tx0, rtc1, tx1 = values
    rtc_s = ((rtc1 - rtc0) % RTC_WRAP) / RTC_HZ
    rate = (tx1 - tx0) / seconds
    print(f"host time        {seconds:.1f} s")
    print(f"RTC1 time        {rtc_s:.2f} s  -> LF clock error {100 * (rtc_s / seconds - 1):+.2f} %")
    print(f"TX events        {tx1 - tx0}  -> {rate:.3f} Hz (nominal {NOMINAL_RATE_HZ:.3f}, "
          f"error {100 * (rate / NOMINAL_RATE_HZ - 1):+.2f} %)")


if __name__ == "__main__":
    main()
