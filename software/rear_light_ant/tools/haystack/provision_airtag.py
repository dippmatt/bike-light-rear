#!/usr/bin/env python3
"""Write the Find My key store of a light that clones an AirTag (ADR-0012).

The light derives the AirTag's rotating keys itself (src/findmy_keys.c) and
only needs a starting point: the key index for "now", the AirTag's shared
secrets at that point (SK of the 15-minute main chain, SKS of the daily
backup chain) and its PUBLIC master key P0. This writes that
provision block as an Intel HEX for the key store pages (KEY_STORE_ADDR in
src/config.h); `make flash-keys` flashes it. The private master key never
leaves this machine.

Run it again to resync a light that lags far behind (after a long time with
a flat battery): it always starts the light at the current index.

Needs: pip install findmy (same venv as track.py)

Usage:
  provision_airtag.py ACCESSORY.json OUT.hex

  ACCESSORY.json  a single FindMyAccessory, as saved by os-findmy-e's
                  tools/machoadi/tag_keys.py --tags-save (one entry picked out)
  OUT.hex         output; contains key material, delete it after flashing
"""

from __future__ import annotations

import argparse
import datetime
import os
import struct
import sys
import zlib
from pathlib import Path

# Must match src/config.h (KEY_STORE_ADDR) and src/key_store.c
KEY_STORE_ADDR = 0x0007D000
PAGE_SIZE = 4096
PAGES = 3
PROVISION_MAGIC = b"FMP2"
KEYS_PER_DAY = 96  # main-chain steps per backup-chain step


def backup_index(index: int) -> int:
    """findmy_secondary_index() in src/findmy_keys.h: the lower of FindMy.py's
    two candidates (_secondary_keys_at), so the light never runs ahead."""
    return index // KEYS_PER_DAY + 1


def provision_block(index: int, master_pub: bytes, sk: bytes, sks: bytes) -> bytes:
    """Layout of struct provision in src/key_store.c, little-endian."""
    assert len(master_pub) == 56 and len(sk) == 32 and len(sks) == 32
    body = PROVISION_MAGIC + struct.pack("<I", index) + master_pub + sk + sks
    return body + struct.pack("<I", zlib.crc32(body))


def intel_hex(addr: int, data: bytes) -> str:
    lines = []

    def record(rtype: int, a: int, payload: bytes) -> None:
        raw = bytes([len(payload), (a >> 8) & 0xFF, a & 0xFF, rtype]) + payload
        lines.append(":" + (raw + bytes([(-sum(raw)) & 0xFF])).hex().upper())

    upper = None
    for off in range(0, len(data), 16):
        a = addr + off
        if a >> 16 != upper:
            upper = a >> 16
            record(0x04, 0, struct.pack(">H", upper))
        record(0x00, a & 0xFFFF, data[off : off + 16])
    record(0x01, 0, b"")
    return "\n".join(lines) + "\n"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("accessory", type=Path)
    ap.add_argument("out", type=Path)
    args = ap.parse_args()

    from cryptography.hazmat.primitives.asymmetric import ec
    from findmy.accessory import FindMyAccessory

    acc = FindMyAccessory.from_json(args.accessory)
    now = datetime.datetime.now(datetime.timezone.utc)
    # The newest index Apple's side expects now; the light then rotates a
    # little slower than 15 min, so it can only fall behind this, never lead.
    index = acc.get_max_index(now)

    # No public accessor for the primary chain's SK; _get_sk is its own
    # generator, pinned to this findmy version (see tools/haystack/README.md).
    sk = acc._primary_gen._get_sk(index)  # noqa: SLF001
    j = backup_index(index)
    sks = acc._secondary_gen._get_sk(j)  # noqa: SLF001

    d = int.from_bytes(acc.master_key, "big")
    pub = ec.derive_private_key(d, ec.SECP224R1()).public_key().public_numbers()
    master_pub = pub.x.to_bytes(28, "big") + pub.y.to_bytes(28, "big")

    # The keys the light will send first, for the BLE scanner check
    want = acc._primary_key_at(index).adv_key_bytes  # noqa: SLF001
    want_backup = acc._secondary_gen[j].adv_key_bytes  # noqa: SLF001
    assert len(want) == 28 and len(want_backup) == 28
    assert want_backup in {k.adv_key_bytes for k in acc._secondary_keys_at(index)}  # noqa: SLF001

    # Provision page, then the two log pages blank: re-provisioning starts a
    # fresh log (flashed with --sectorerase, the 0xFF pages erase them)
    image = provision_block(index, master_pub, sk, sks)
    image += b"\xff" * (PAGES * PAGE_SIZE - len(image))

    fd = os.open(args.out, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o600)
    with os.fdopen(fd, "w") as f:
        f.write(intel_hex(KEY_STORE_ADDR, image))

    print(f"{acc.name or 'accessory'}: key index {index}, backup index {j} at {now.isoformat(timespec='seconds')}")
    for label, k in (("main", want), ("backup", want_backup)):
        print(f"{label:6} key starts {k[:3].hex()}... (BLE scanner: address {k[0] | 0xC0:02x}{k[1:6].hex()})")
    print(f"wrote {args.out} (key material: delete after flashing)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
