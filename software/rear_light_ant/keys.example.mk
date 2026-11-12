# Copy this file to keys.local.mk (git-ignored) and fill in both keys.
# The firmware build (make, make flash) stops until they are set. make test
# does not need them. See README.md, "Keys".

# ANT+ network key, 16 hex digits (8 bytes), e.g. 0123456789ABCDEF (dummy).
# Garmin publishes it only to ANT+ Adopters; you may not publish it.
# Source: https://developer.garmin.com/ant-program/ant-ant-plus/network-keys/
ANT_PLUS_NETWORK_KEY := ENTER-16-HEX-DIGITS

# ANT SoftDevice (S212) licence key, format xxxx-xxxx-xxxx-xxxx-xxxx-xxxx-xxxx-xxxx.
# The evaluation key is in the nRF SDK / ANT stack headers (nrf_sdm.h) and is for
# non-commercial evaluation only. A commercial product needs its own licence:
# https://developer.garmin.com/ant-program/licensing/
ANT_LICENSE_KEY := ENTER-LICENSE-KEY

# Optional: Find My advert so the light can be found through Apple's network
# (ADR-0011, tools/haystack/README.md). One key per light, the "Advertisement
# key" line of the .keys file from generate_keys.py: base64 of 28 bytes.
# Leave unset to build without the advert. Never commit the private key.
# HAYSTACK_KEY_REAR  := ENTER-BASE64
# HAYSTACK_KEY_FRONT := ENTER-BASE64
#
# Or: send a cloned AirTag's rotating keys instead (ADR-0012), shown in the
# Find My app. Path of the accessory JSON (keep it in tools/haystack/data/,
# git-ignored); `make flash` / `make flash-keys` provision the light from it.
# Takes over from HAYSTACK_KEY_<LIGHT>.
# HAYSTACK_AIRTAG_REAR  := tools/haystack/data/ENTER-NAME.json
# HAYSTACK_AIRTAG_FRONT := tools/haystack/data/ENTER-NAME.json
