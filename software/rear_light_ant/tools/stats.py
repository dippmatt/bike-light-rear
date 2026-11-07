#!/usr/bin/env python3
"""Read struct proto_stats from the running target over SWD and print it.

usage: stats.py <elf> <nm>
"""
import re
import struct
import subprocess
import sys
import tempfile

LOG_LEN = 8
FORMAT = "<13I8s8sBB2x66H3I" + "IB3x8s" * LOG_LEN + "4I8B3iiIHBB4II"
STATE_NAMES = {0: "Off", 1: "Steady", 2: "Flash", 3: "Smart", 4: "Idle Charging",
               5: "Smart Auto-Off", 6: "Bright"}
EVENT_NAMES = {
    0x01: "RX_SEARCH_TIMEOUT", 0x02: "RX_FAIL", 0x03: "TX", 0x04: "TRANSFER_RX_FAILED",
    0x05: "TRANSFER_TX_COMPLETED", 0x06: "TRANSFER_TX_FAILED", 0x07: "CHANNEL_CLOSED",
    0x08: "RX_FAIL_GO_TO_SEARCH", 0x09: "CHANNEL_COLLISION", 0x0A: "TRANSFER_TX_START",
    0x0B: "RX_DATA_OVERFLOW", 0x35: "QUE_OVERFLOW", 0x40: "RX (0x80)", 0x41: "other",
}
PAGE_NAMES = {
    1: "light state", 2: "capabilities", 19: "supplementary", 32: "disconnect",
    33: "connect", 34: "light settings", 70: "request", 80: "manufacturer", 81: "product",
}
MSG_NAMES = {0x4E: "broadcast", 0x4F: "acknowledged"}
FIELDS = [
    "magic", "uptime_s", "tx_events", "rx_messages", "rx_commands",
    "rx_requests", "mode_changes", "battery_mv", "last_error",
    "last_error_site", "sd_fault_id", "sd_fault_pc", "sd_fault_info",
]
MODES = {0: "Off", 1: "Steady (front: Standard)", 2: "Flash", 3: "Smart", 4: "Bright"}


def symbol_address(elf, nm):
    out = subprocess.run([nm, elf], check=True, capture_output=True, text=True).stdout
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[2] == "proto_stats":
            return int(parts[0], 16)
    sys.exit("proto_stats not found in " + elf)


def read_memory(address, size):
    """Read target RAM through a J-Link session without halting or resetting the CPU.

    (`nrfjprog --memrd` halts the core and `nrfjprog --run` then restarts it
    from reset, which wipes the counters and drops the ANT link.)
    """
    words = (size + 3) // 4
    with tempfile.NamedTemporaryFile("w", suffix=".jlink") as f:
        f.write(f"connect\nmem32 0x{address:08X} {words}\nexit\n")
        f.flush()
        out = subprocess.run(
            ["JLinkExe", "-device", "NRF52833_XXAA", "-if", "SWD", "-speed", "4000",
             "-autoconnect", "1", "-NoGui", "1", "-CommandFile", f.name],
            check=True, capture_output=True, text=True,
        ).stdout
    data = bytearray()
    for line in out.splitlines():
        match = re.match(r"^[0-9A-Fa-f]{8} = ((?:[0-9A-Fa-f]{8}\s*)+)$", line)
        if match:
            for word in match.group(1).split():
                data += int(word, 16).to_bytes(4, "little")
    if len(data) < size:
        sys.exit("could not read target memory:\n" + out)
    return bytes(data[:size])


def sample(address):
    return struct.unpack(FORMAT, read_memory(address, struct.calcsize(FORMAT)))


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    address = symbol_address(sys.argv[1], sys.argv[2])
    values = sample(address)
    stats = dict(zip(FIELDS, values[:13]))
    last_rx, last_tx, mode, light_index = values[13], values[14], values[15], values[16]
    events = values[17:17 + 0x42]
    rx_other, rx_other_id, rx_next = values[17 + 0x42:17 + 0x42 + 3]
    log_raw = values[17 + 0x42 + 3:17 + 0x42 + 3 + 3 * LOG_LEN]
    (events_dropped, reset_reason, ant_init_error, silence_timeouts,
     state, sleeping, usb, low_batt, accel_ok, light_ok, channel_state, parked, ax, ay, az, millilux, blanks,
     rear_dev, rear_phase, rear_cmd, rear_sent, rear_acked, rear_failed, rear_excl, _inject) = values[17 + 0x42 + 3 + 3 * LOG_LEN:]
    log = [log_raw[i:i + 3] for i in range(0, len(log_raw), 3)]

    if stats["magic"] != 0x414E5450:
        print("magic mismatch: firmware not running or wrong image")
    uptime = stats["uptime_s"]
    for name in FIELDS[1:]:
        print(f"{name:16} {stats[name]}")
    if uptime:
        print(f"{'tx/uptime [Hz]':16} {stats['tx_events'] / uptime:.3f} (expect 8.02)")
    print(f"{'mode':16} {MODES.get(mode, mode)}")
    print(f"{'state':16} {STATE_NAMES.get(state, state)}{' (sleeping)' if sleeping else ''}"
          f"{' (parked)' if parked else ''}")
    print(f"{'usb / low batt':16} {usb} / {low_batt}")
    print(f"{'sensors ok':16} accel {accel_ok}, light {light_ok}")
    print(f"{'accel [m/s2]':16} x {ax / 1000:.2f}  y {ay / 1000:.2f}  z {az / 1000:.2f}")
    print(f"{'ambient [lux]':16} {millilux / 1000:.2f} ({blanks} blanked measurements)")
    print(f"{'ant channel':16} {['closed', 'open', 'closing'][channel_state] if channel_state < 3 else channel_state}"
          f", init error {ant_init_error:#x}, silence timeouts {silence_timeouts}")
    print(f"{'reset reason':16} {reset_reason:#x} (0 = power-on), events dropped {events_dropped}")
    print(f"{'light index':16} {light_index} (0 unconnected, 1 connected)")
    rear_cmd_text = "off" if rear_cmd == 1 else f"mode {rear_cmd >> 2}"
    print(f"{'rear control':16} {['searching', 'checking', 'found'][rear_phase] if rear_phase < 3 else rear_phase}"
          f" {rear_dev:#06x}, last {rear_cmd_text}, sent {rear_sent} acked {rear_acked}"
          f" failed {rear_failed} excluded {rear_excl} (front light only)")
    print(f"{'last rx page':16} {last_rx.hex(' ')}")
    print(f"{'last tx page':16} {last_tx.hex(' ')}")
    print("ANT events:")
    for code, count in enumerate(events):
        if count:
            print(f"  {EVENT_NAMES.get(code, hex(code)):24} {count}")
    print(f"rx neither broadcast nor acknowledged: {rx_other} (last id {rx_other_id:#x})")
    print("received messages, oldest first (seq, type, data):")
    for seq, msg_id, data in sorted(e for e in log if e[1] != 0 and e[0] < rx_next):
        name = PAGE_NAMES.get(data[0], "unknown page")
        print(f"  #{seq:<4} {MSG_NAMES.get(msg_id, hex(msg_id)):12} {data.hex(' ')}  ({name})")


if __name__ == "__main__":
    main()
