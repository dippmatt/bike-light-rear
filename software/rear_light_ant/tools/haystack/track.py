#!/usr/bin/env python3
"""Find the light through Apple's Find My network with FindMy.py (ADR-0011).

  track.py login           sign in with a spare Apple ID once (SMS or trusted device code)
  track.py fetch           print the reported positions of output/*_devices.json, newest first

Needs: pip install findmy  (0.10.2 or newer: older versions fail with HTTP 503)
The login session is kept in data/ (git-ignored). Do not share data/ or output/.
"""

import getpass
import glob
import json
import sys
from pathlib import Path

from findmy import (
    AppleAccount,
    KeyPair,
    LocalAnisetteProvider,
    LoginState,
    SmsSecondFactorMethod,
    TrustedDeviceSecondFactorMethod,
)

HERE = Path(__file__).resolve().parent
DATA = HERE / "data"
ACCOUNT = DATA / "account.json"
ANISETTE_LIBS = DATA / "ani_libs.bin"
DEVICES = sorted(glob.glob(str(HERE / "output" / "*_devices.json")))


def login() -> None:
    DATA.mkdir(exist_ok=True)
    acc = AppleAccount(LocalAnisetteProvider(libs_path=str(ANISETTE_LIBS)))
    email = input("Apple ID (use a spare one): ")
    state = acc.login(email, getpass.getpass("Password: "))

    if state == LoginState.REQUIRE_2FA:
        methods = acc.get_2fa_methods()
        for i, method in enumerate(methods):
            if isinstance(method, TrustedDeviceSecondFactorMethod):
                print(f"{i} - code shown on a trusted Apple device")
            elif isinstance(method, SmsSecondFactorMethod):
                print(f"{i} - SMS to {method.phone_number}")
        method = methods[int(input("Method: "))]
        method.request()
        method.submit(input("Code: "))

    acc.to_json(str(ACCOUNT))
    print(f"Logged in as {acc.account_name}; session saved to {ACCOUNT}")


def load_devices() -> list[tuple[str, KeyPair]]:
    if not DEVICES:
        sys.exit("No output/*_devices.json: run generate_keys.py first (README.md)")
    devices = []
    for path in DEVICES:
        for device in json.load(open(path)):
            devices.append((device["name"], KeyPair.from_b64(device["privateKey"])))
    return devices


def fetch() -> None:
    acc = AppleAccount.from_json(str(ACCOUNT), anisette_libs_path=str(ANISETTE_LIBS))
    for name, key in load_devices():
        reports = sorted(acc.fetch_location_history(key), key=lambda r: r.timestamp, reverse=True)
        print(f"{name}: {len(reports)} reports")
        for r in reports:
            when = r.timestamp.astimezone().strftime("%Y-%m-%d %H:%M:%S")
            print(f"  {when}  {r.latitude:.6f}, {r.longitude:.6f}  (+-{r.horizontal_accuracy} m)  "
                  f"https://www.openstreetmap.org/?mlat={r.latitude}&mlon={r.longitude}#map=17/{r.latitude}/{r.longitude}")
        if not reports:
            print("  none yet: the light must pass iPhones, the first report can take 30 minutes or more")
    acc.to_json(str(ACCOUNT))


def main() -> int:
    DATA.mkdir(exist_ok=True)
    cmd = sys.argv[1] if len(sys.argv) > 1 else ""
    if cmd == "login":
        login()
    elif cmd == "fetch":
        fetch()
    else:
        print(__doc__)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
