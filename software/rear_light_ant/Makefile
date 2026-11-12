# ANT+ Bike Lights firmware for the rear and the front light (nRF52833, ANT S212 7.0.1)
#
# Builds with the GCC that ships with the nRF Connect SDK toolchain; no nRF5
# SDK is needed (the SoftDevice API headers, the nRF52833 MDK and CMSIS are
# the only external sources).
#
#   make            build build/rear/light_rear.hex (rear light, the default)
#   make LIGHT=front  the same for the front light (build/front/light_front.hex)
#   make test       host unit tests of both variants
#   make flash      erase chip, program SoftDevice and application, reset
#   make flash-app  program the application only
#   make stats      read the debug counters over SWD
#   make rate       measure TX rate and LF clock error (RATE_S seconds)
#
# Override on the command line if your paths differ:
#   make NCS=~/ncs/v3.1.1 TOOLCHAIN=~/ncs/toolchains/<id>/opt/zephyr-sdk/arm-zephyr-eabi/bin

# make V=1 shows the full commands
Q := $(if $(V),,@)

NCS       ?= $(HOME)/ncs/v3.1.1
TOOLCHAIN ?= $(HOME)/ncs/toolchains/b2ecd2435d/opt/zephyr-sdk/arm-zephyr-eabi/bin
CROSS     ?= $(TOOLCHAIN)/arm-zephyr-eabi-

CC      := $(CROSS)gcc
OBJCOPY := $(CROSS)objcopy
SIZE    := $(CROSS)size
NM      := $(CROSS)nm

MDK     := $(NCS)/modules/hal/nordic/nrfx/mdk
CMSIS   := $(NCS)/modules/hal/cmsis/CMSIS/Core/Include

# S212 7.0.1 SoftDevice, extracted from the downloaded archive (not committed)
SD_ZIP  ?= ../ant_source/SoftDevices_Current_Items/ANT_s212_nrf52_7.0.1.zip
SD_DIR  := build/sd
SD_NAME := ANT_s212_nrf52_7.0.1
SD_API  := $(SD_DIR)/$(SD_NAME)/$(SD_NAME).API/include
SD_HEX  := $(SD_DIR)/$(SD_NAME)/$(SD_NAME).hex

# Light variant (ADR-0009): rear (default) or front. Every target that builds,
# flashes or reads a board takes it, e.g. make LIGHT=front flash
LIGHT ?= rear
ifeq ($(LIGHT),front)
EXTRA_DEFS += -DLIGHT_FRONT=1
else ifneq ($(LIGHT),rear)
$(error LIGHT must be rear or front)
endif

# Build output of this variant; the SoftDevice in build/sd is shared
B   := build/$(LIGHT)
APP := $(B)/light_$(LIGHT)

# Keys are not in the repository. Copy keys.example.mk to keys.local.mk (git-
# ignored) and fill in ANT_PLUS_NETWORK_KEY and ANT_LICENSE_KEY; see README.md,
# "Keys". The firmware build stops if they are missing.
-include keys.local.mk

# ANT+ network key as a C byte list, e.g. 0x01,0x23,... (written to $(B)/ant_keys.h)
ANT_PLUS_NETWORK_KEY_BYTES := $(shell echo '$(ANT_PLUS_NETWORK_KEY)' | grep -E '^[0-9A-Fa-f]{16}$$' | sed 's/../0x&,/g; s/,$$//')

# Find My advert (ADR-0011/0012), optional. Two mutually exclusive sources,
# checked in this order:
#   1. HAYSTACK_AIRTAG_REAR / HAYSTACK_AIRTAG_FRONT in keys.local.mk: path of
#      a cloned AirTag's accessory JSON. The light derives the AirTag's
#      rotating keys itself from a key store that `make flash-keys` writes
#      (ADR-0012). Takes over from HAYSTACK_KEY_<LIGHT>.
#   2. HAYSTACK_KEY_REAR / HAYSTACK_KEY_FRONT in keys.local.mk: our own fixed
#      OpenHaystack key (base64, 28 bytes).
# Without either, the advert is compiled out.
HAYSTACK_AIRTAG := $(HAYSTACK_AIRTAG_$(shell echo $(LIGHT) | tr a-z A-Z))
HAYSTACK_KEY := $(HAYSTACK_KEY_$(shell echo $(LIGHT) | tr a-z A-Z))
# micro-ecc with only P-224 and its bignum API (findmy_keys.c); same for host tests
UECC_DEFS := -DuECC_ENABLE_VLI_API=1 -DuECC_SUPPORT_COMPRESSED_POINT=0 \
             -DuECC_SUPPORTS_secp160r1=0 -DuECC_SUPPORTS_secp192r1=0 \
             -DuECC_SUPPORTS_secp256r1=0 -DuECC_SUPPORTS_secp256k1=0
CRYPTO_INC := -Isrc/ext/tinycrypt/include
CRYPTO_SRCS := src/ext/micro-ecc/uECC.c src/ext/tinycrypt/source/sha256.c \
               src/ext/tinycrypt/source/utils.c
ifneq ($(HAYSTACK_AIRTAG),)
EXTRA_DEFS += -DHAYSTACK=1 -DAIRTAG_ROLLING=1 $(UECC_DEFS) $(CRYPTO_INC)
HAYSTACK_SRCS := src/haystack_adv.c src/haystack.c src/findmy_keys.c src/key_store.c $(CRYPTO_SRCS)
KEY_HEADERS := $(B)/ant_keys.h
else ifneq ($(HAYSTACK_KEY),)
HAYSTACK_KEY_LEN_BYTES := $(shell echo '$(HAYSTACK_KEY)' | base64 -d 2>/dev/null | wc -c)
HAYSTACK_KEY_BYTES := $(shell echo '$(HAYSTACK_KEY)' | base64 -d 2>/dev/null | od -An -v -tx1 | tr -s ' \n' ' ' | sed 's/^ //; s/ $$//; s/ /,0x/g; s/^/0x/')
EXTRA_DEFS += -DHAYSTACK=1
HAYSTACK_SRCS := src/haystack_adv.c src/haystack.c
KEY_HEADERS := $(B)/ant_keys.h $(B)/haystack_key.h
else
KEY_HEADERS := $(B)/ant_keys.h
endif

# Low-frequency clock source: rc (default, no crystal on this board),
# synth (derived from the 32 MHz crystal, costs current) or xtal (32.768 kHz
# crystal on XL1/XL2, needs a board change)
LFCLK ?= rc
EXTRA_DEFS += -DLFCLK_$(shell echo $(LFCLK) | tr a-z A-Z)=1

# make BENCH=1: board powered from the debugger without a battery. The battery
# reading is replaced by a fixed value so the critical cut-off cannot fire.
# NEVER flash such an image to a light that is used on a bike.
ifeq ($(BENCH),1)
EXTRA_DEFS += -DBENCH_BUILD
endif

# make FORCE_SD_FAILURE=1: pretend the SoftDevice cannot be enabled (tests SYS-5)
ifeq ($(FORCE_SD_FAILURE),1)
EXTRA_DEFS += -DFORCE_SD_FAILURE
endif

CFLAGS := -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -Os -g3 \
          -std=gnu11 -ffreestanding -fno-builtin -fno-tree-loop-distribute-patterns \
          -ffunction-sections -fdata-sections -fno-common \
          -Wall -Wextra -Werror -Wshadow \
          -DNRF52833_XXAA -DANT_LICENSE_KEY=\"$(ANT_LICENSE_KEY)\" $(EXTRA_DEFS) \
          -Isrc -I$(B) -isystem $(MDK) -isystem $(CMSIS) -isystem $(SD_API) -isystem $(SD_API)/nrf52

LDFLAGS := -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -nostartfiles -nostdlib \
           -Tlinker/rear_light_ant.ld -Wl,--gc-sections -Wl,-Map=$(APP).map

SRCS := src/startup.c src/main.c src/ant_link.c src/bike_light.c src/app_events.c \
        src/sm.c src/detectors.c src/battery_logic.c src/platform.c src/light.c \
        src/status_led.c src/soft_timer.c src/button.c src/battery.c src/power.c \
        src/sensors.c src/twim.c src/rear_link.c $(HAYSTACK_SRCS)
OBJS := $(SRCS:src/%.c=$(B)/%.o) $(B)/system_nrf52833.o

.PHONY: all clean test flash flash-app flash-keys stats rate watch inject size sd

all: $(APP).hex size check-vectors

sd: $(SD_HEX)

$(SD_HEX): $(SD_ZIP)
	mkdir -p $(SD_DIR)/$(SD_NAME)
	unzip -q -o $(SD_ZIP) -d $(SD_DIR)/$(SD_NAME)
	touch $@

# Rebuild everything when the compiler flags (LFCLK, licence key) change
$(B)/.flags: FORCE | $(B)
	@echo '$(CFLAGS)' | cmp -s - $@ || echo '$(CFLAGS)' > $@
FORCE:

# Generated from keys.local.mk; stops the build if a key is missing or malformed.
# Rewritten only when the content changes, so ant_link.o is not rebuilt needlessly.
$(B)/ant_keys.h: FORCE | $(B)
	@test -n '$(ANT_LICENSE_KEY)' || { echo 'ANT_LICENSE_KEY is not set: see README.md, "Keys"'; exit 1; }
	@test -n '$(ANT_PLUS_NETWORK_KEY_BYTES)' || { echo 'ANT_PLUS_NETWORK_KEY must be 16 hex digits: see README.md, "Keys"'; exit 1; }
	@echo '#define ANT_PLUS_NETWORK_KEY_BYTES $(ANT_PLUS_NETWORK_KEY_BYTES)' | cmp -s - $@ || echo '#define ANT_PLUS_NETWORK_KEY_BYTES $(ANT_PLUS_NETWORK_KEY_BYTES)' > $@

# Same for the Find My key (only when the variant has one; otherwise a stale
# dependency on it from an earlier fixed-key build must not run the check)
ifneq ($(filter $(B)/haystack_key.h,$(KEY_HEADERS)),)
$(B)/haystack_key.h: FORCE | $(B)
	@test '$(HAYSTACK_KEY_LEN_BYTES)' = 28 || { echo 'HAYSTACK_KEY_$(shell echo $(LIGHT) | tr a-z A-Z) must be base64 of 28 bytes: see README.md, "Keys"'; exit 1; }
	@echo '#define HAYSTACK_KEY_BYTES $(HAYSTACK_KEY_BYTES)' | cmp -s - $@ || echo '#define HAYSTACK_KEY_BYTES $(HAYSTACK_KEY_BYTES)' > $@
endif

$(B)/%.o: src/%.c $(SD_HEX) $(B)/.flags $(KEY_HEADERS) | $(B)
	@echo "  CC      $<"
	$(Q)$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Vendored libraries (src/ext): compiled as shipped, without our -Werror set
$(B)/ext/%.o: src/ext/%.c $(SD_HEX) $(B)/.flags | $(B)
	@echo "  CC      $<"
	@mkdir -p $(@D)
	$(Q)$(CC) $(filter-out -Wall -Wextra -Werror -Wshadow,$(CFLAGS)) -w -MMD -MP -c $< -o $@

# The MDK's system file is third-party code: no -Werror/-Wshadow for it
$(B)/system_nrf52833.o: $(MDK)/system_nrf52833.c $(B)/.flags | $(B)
	@echo "  CC      $<"
	$(Q)$(CC) $(filter-out -Werror -Wshadow,$(CFLAGS)) -c $< -o $@

$(B) build/test:
	mkdir -p $@

$(APP).elf: $(OBJS) linker/rear_light_ant.ld
	@echo "  LD      $@"
	$(Q)$(CC) $(LDFLAGS) $(OBJS) -lgcc -o $@

$(APP).hex: $(APP).elf
	@echo "  HEX     $@"
	$(Q)$(OBJCOPY) -O ihex $< $@

# The interrupt handlers the application implements must replace the weak
# defaults in the vector table (a silent name mismatch would leave an
# interrupt without a handler)
.PHONY: check-vectors
check-vectors: $(APP).elf
	@for h in SWI2_EGU2_IRQHandler RTC1_IRQHandler GPIOTE_IRQHandler; do \
	    $(NM) $< | grep -q " T $$h$$" || { echo "missing handler $$h"; exit 1; }; \
	done

size: $(APP).elf
	$(SIZE) $<

# Both variants, so a change for one light cannot break the other unnoticed
TEST_BINS := $(foreach v,rear front,build/test/test_bike_light_$(v) build/test/test_core_$(v)) \
             build/test/test_rear_link build/test/test_haystack_adv \
             build/test/test_findmy_keys build/test/test_key_store

test: $(TEST_BINS)
	@for t in $(TEST_BINS); do echo "  RUN     $$t"; $$t || exit 1; done

HOST_CFLAGS := -Wall -Wextra -Werror -Wshadow -std=c11 -fsanitize=address,undefined -g

variant_def = $(if $(filter front,$(1)),-DLIGHT_FRONT=1,)

build/test/test_bike_light_%: test/test_bike_light.c src/bike_light.c $(wildcard src/*.h) | build/test
	gcc $(HOST_CFLAGS) $(call variant_def,$*) -o $@ test/test_bike_light.c src/bike_light.c

# Rear control of the front light, tested against the rear light's command
# handler (rear variant of bike_light.c), see test/test_rear_link.c
build/test/test_rear_link: test/test_rear_link.c src/rear_link.c src/bike_light.c $(wildcard src/*.h) | build/test
	gcc $(HOST_CFLAGS) -o $@ test/test_rear_link.c src/rear_link.c src/bike_light.c

build/test/test_haystack_adv: test/test_haystack_adv.c src/haystack_adv.c $(wildcard src/*.h) | build/test
	gcc $(HOST_CFLAGS) -o $@ test/test_haystack_adv.c src/haystack_adv.c

# The vendored crypto is built without -Werror here too (third-party code)
build/test/test_findmy_keys: test/test_findmy_keys.c src/findmy_keys.c $(CRYPTO_SRCS) $(wildcard src/*.h) | build/test
	gcc $(HOST_CFLAGS) $(UECC_DEFS) $(CRYPTO_INC) -Isrc -c -o $@.o src/findmy_keys.c
	gcc $(filter-out -Wall -Wextra -Werror -Wshadow,$(HOST_CFLAGS)) -w $(UECC_DEFS) $(CRYPTO_INC) \
	    -o $@ test/test_findmy_keys.c $@.o $(CRYPTO_SRCS)

build/test/test_key_store: test/test_key_store.c src/key_store.c $(wildcard src/*.h) | build/test
	gcc $(HOST_CFLAGS) -o $@ test/test_key_store.c src/key_store.c

CORE_SRCS := src/sm.c src/app_events.c src/battery_logic.c src/detectors.c src/bike_light.c

build/test/test_core_%: test/test_core.c test/test_platform.c $(CORE_SRCS) $(wildcard src/*.h) | build/test
	gcc $(HOST_CFLAGS) $(call variant_def,$*) -o $@ test/test_core.c test/test_platform.c $(CORE_SRCS)

# Chip erase also clears UICR; the application hex restores NFCPINS = GPIO.
# It also clears the Find My key store, so an AirTag build re-provisions it.
flash: $(APP).hex $(SD_HEX)
	nrfjprog --family NRF52 --program $(SD_HEX) --chiperase --verify
	nrfjprog --family NRF52 --program $(APP).hex --sectoranduicrerase --verify
	$(if $(HAYSTACK_AIRTAG),$(MAKE) --no-print-directory flash-keys)
	nrfjprog --family NRF52 --reset

# Find My key store (ADR-0012): the AirTag's key position for "now", from
# the accessory JSON in HAYSTACK_AIRTAG_<LIGHT>. Also the way to resync a
# light that lags too far behind. The hex holds key material: deleted after.
HAYSTACK_PY ?= tools/haystack/.venv/bin/python
flash-keys:
	@test -n '$(HAYSTACK_AIRTAG)' || { echo 'HAYSTACK_AIRTAG_$(shell echo $(LIGHT) | tr a-z A-Z) is not set: see keys.example.mk'; exit 1; }
	@mkdir -p $(B)
	$(HAYSTACK_PY) tools/haystack/provision_airtag.py '$(HAYSTACK_AIRTAG)' $(B)/key_store.hex
	nrfjprog --family NRF52 --program $(B)/key_store.hex --sectorerase --verify; \
	    status=$$?; rm -f $(B)/key_store.hex; exit $$status
	nrfjprog --family NRF52 --reset

flash-app: $(APP).hex
	nrfjprog --family NRF52 --program $(APP).hex --sectoranduicrerase --verify
	nrfjprog --family NRF52 --reset

# RATE_S: measurement time of `make rate`
RATE_S ?= 30
stats:
	python3 tools/stats.py $(APP).elf $(NM)

# TX rate and LF clock error over RATE_S seconds, CPU keeps running (J-Link)
rate:
	python3 tools/rate.py $(APP).elf $(NM) $(RATE_S)

# bench builds: post an event, e.g. make inject EVT=BUTTON_LONG or EVT=REMOTE_SET_STATE ARG=1
inject:
	python3 tools/inject.py $(APP).elf $(NM) $(EVT) $(ARG)

# log counter changes for WATCH_S seconds (reconnect tests)
WATCH_S ?= 300
watch:
	python3 tools/watch.py $(APP).elf $(NM) $(WATCH_S)

clean:
	rm -rf build

-include $(OBJS:.o=.d)
