# Finding the light through Apple's Find My network

The light sends a Find My advert (ADR-0011). Any iPhone that passes it uploads its own position, encrypted for your key. `track.py` fetches those reports from Apple and decrypts them with your private key. You never connect to the light, and it does **not** show up in the Find My app.

This is the unofficial OpenHaystack route, a proof of concept. Use a **spare Apple ID**, not your main one; Apple can change or block it at any time.

What to expect: no live position. A new key can take 30 minutes or more to show its first report, and you only get positions where an iPhone passed the light. A bike in a basement or a field shows nothing. The position is the passing phone's, so tens of metres off at best. For the last metres use a BLE scanner app (nRF Connect): the light advertises every 2 s.

```
light ──BLE advert──> strangers' iPhones ──encrypted──> Apple
                                                          │
 track.py fetch  <── decrypt with private key  <──────────┘
```

> **Status (10 Nov 2026): reading the position back is blocked.** The light's side works (the advert was seen on air, byte-identical to the key). But since 12 Oct 2026 Apple only gives the token that unlocks the reports to clients with a real macOS/iOS identity. With the built-in anisette on Linux/WSL/Windows, `track.py login` fails with `This action cannot be completed at this time` (`-80045`), or login works and the report fetch returns 401. Upstream: [FindMy.py issue 278](https://github.com/malmeloo/FindMy.py/issues/278), and the maintainer said on 28 Oct that the fix is unfinished. Reported workarounds: a real Mac with native anisette, or emulating macOS's anisette code on Linux (`sacca97/os-findmy-e`). A stock or sideloaded iPad cannot do it: apps cannot call Apple's private AuthKit/ADI code. Do not retry the login in a loop; failed attempts can get an Apple ID flagged.

Why FindMy.py and not macless-haystack: in September 2026 Apple started answering 503 to the sign-in of clients that identify as Xcode. The macless-haystack Docker image still does ([issues 245 and 249](https://github.com/dchristl/macless-haystack/issues/245)); FindMy.py fixed it in 0.10.2.

## 1. Make the key (once per light)

`generate_keys.py` is upstream's tool from macless-haystack, not copied into this repo ([releases](https://github.com/dchristl/macless-haystack/releases), v2.7.2 here).

```sh
cd tools/haystack
python3 -m venv .venv && . .venv/bin/activate && pip install findmy cryptography
curl -LO https://github.com/dchristl/macless-haystack/releases/download/v2.7.2/generate_keys.py
python3 generate_keys.py -p rear
```

It writes `output/rear.keys` and `output/rear_devices.json` (it deletes `output/` first). Keep the **private key** secret and backed up: without it the reports cannot be decrypted, and it cannot be recovered from the light.

## 2. Put the advertisement key into the firmware

Copy the `Advertisement key:` line of `output/rear.keys` into `software/rear_light_ant/keys.local.mk` (git-ignored, see `keys.example.mk`):

```make
HAYSTACK_KEY_REAR := <Advertisement key>
```

Then `make flash`. Without the key the advert is left out. A key that is not 28 bytes of base64 stops the build.

Check with a BLE scanner: a non-connectable device with Apple manufacturer data `12 19 00 …`, about every 2 s, address starting with the key's first bytes (`key[0] | 0xC0`, then `key[1..5]`).

## 3. Sign in and read the position

```sh
cd tools/haystack
. .venv/bin/activate
python3 track.py login     # once: Apple ID, password, 2FA code
python3 track.py fetch     # as often as you like
```

`fetch` prints one line per report, newest first: time, latitude, longitude, accuracy and an OpenStreetMap link. The login session stays in `data/account.json`; the password is not stored.

Needs `findmy` 0.10.2 or newer. If sign-in fails with an Apple error code (for example `-80045`), Apple is rejecting this client or account; try a different spare Apple ID, wait, or check the [FindMy.py issues](https://github.com/malmeloo/FindMy.py/issues).

## Alternative: send a cloned AirTag's rotating keys (ADR-0012)

Instead of a fixed key, the light can send the keys of an AirTag from your Apple account. It then shows up in the Find My app as that AirTag, and the key changes every 15 minutes like a real one. You need the AirTag's accessory JSON (one `FindMyAccessory`, for example from `sacca97/os-findmy-e`'s `tag_keys.py --tags-save`); keep it in `data/`.

```make
# keys.local.mk
HAYSTACK_AIRTAG_REAR := tools/haystack/data/<name>.json
```

`make flash` builds the firmware with the on-device key schedule and runs `make flash-keys`: `provision_airtag.py` computes the key index for the current time and writes a key store (public master key, index, shared secret; never the private key) to the last three flash pages. The light counts on from there with its own clock and keeps its place across resets in that flash. It never runs ahead of Apple's schedule, only behind (after a reset or a flat battery); Apple's side catches up when it sees a report. If the light lagged for a long time, `make flash-keys` resyncs it. `make flash-app` keeps the key store.

The script uses FindMy.py's private `_primary_gen._get_sk`, tested with findmy 0.10.2: check it after an upgrade (`make test` covers the firmware side with FindMy.py vectors, `test/test_findmy_keys.c`).

## Privacy and risks

- The key never changes, so anyone who reads the advert can recognise this light and, with the public key, ask Apple for its reports. They cannot decrypt them. iPhones may also show an unknown-tracker alert if the light travels with someone else.
- Treat `data/` and `output/` as secrets. They are git-ignored.
- Apple can block the unofficial API or the spare Apple ID at any time.
