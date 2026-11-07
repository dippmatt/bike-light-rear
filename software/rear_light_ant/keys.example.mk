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
