# ANT+ Bike Lights bench prototype for the rear light (nRF52833, ANT S212 7.0.1)
#
# Builds with the GCC that ships with the nRF Connect SDK toolchain; no nRF5
# SDK is needed (the SoftDevice API headers, the nRF52833 MDK and CMSIS are
# the only external sources).
#
#   make            build build/rear_light_ant.hex
#   make test       host unit tests of the profile logic
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

# Keys are not in the repository. Copy keys.example.mk to keys.local.mk (git-
# ignored) and fill in ANT_PLUS_NETWORK_KEY and ANT_LICENSE_KEY; see README.md,
# "Keys". The firmware build stops if they are missing.
-include keys.local.mk

# ANT+ network key as a C byte list, e.g. 0x01,0x23,... (written to build/ant_keys.h)
ANT_PLUS_NETWORK_KEY_BYTES := $(shell echo '$(ANT_PLUS_NETWORK_KEY)' | grep -E '^[0-9A-Fa-f]{16}$$' | sed 's/../0x&,/g; s/,$$//')

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
          -Isrc -Ibuild -isystem $(MDK) -isystem $(CMSIS) -isystem $(SD_API) -isystem $(SD_API)/nrf52

LDFLAGS := -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -nostartfiles -nostdlib \
           -Tlinker/rear_light_ant.ld -Wl,--gc-sections -Wl,-Map=build/rear_light_ant.map

SRCS := src/startup.c src/main.c src/ant_link.c src/bike_light.c src/app_events.c \
        src/sm.c src/detectors.c src/battery_logic.c src/platform.c src/light.c \
        src/status_led.c src/soft_timer.c src/button.c src/battery.c src/power.c \
        src/sensors.c src/twim.c
OBJS := $(SRCS:src/%.c=build/%.o) build/system_nrf52833.o

.PHONY: all clean test flash flash-app stats rate watch inject size sd

all: build/rear_light_ant.hex size check-vectors

sd: $(SD_HEX)

$(SD_HEX): $(SD_ZIP)
	mkdir -p $(SD_DIR)/$(SD_NAME)
	unzip -q -o $(SD_ZIP) -d $(SD_DIR)/$(SD_NAME)
	touch $@

# Rebuild everything when the compiler flags (LFCLK, licence key) change
build/.flags: FORCE | build
	@echo '$(CFLAGS)' | cmp -s - $@ || echo '$(CFLAGS)' > $@
FORCE:

# Generated from keys.local.mk; stops the build if a key is missing or malformed.
# Rewritten only when the content changes, so ant_link.o is not rebuilt needlessly.
build/ant_keys.h: FORCE | build
	@test -n '$(ANT_LICENSE_KEY)' || { echo 'ANT_LICENSE_KEY is not set: see README.md, "Keys"'; exit 1; }
	@test -n '$(ANT_PLUS_NETWORK_KEY_BYTES)' || { echo 'ANT_PLUS_NETWORK_KEY must be 16 hex digits: see README.md, "Keys"'; exit 1; }
	@echo '#define ANT_PLUS_NETWORK_KEY_BYTES $(ANT_PLUS_NETWORK_KEY_BYTES)' | cmp -s - $@ || echo '#define ANT_PLUS_NETWORK_KEY_BYTES $(ANT_PLUS_NETWORK_KEY_BYTES)' > $@

build/%.o: src/%.c $(SD_HEX) build/.flags build/ant_keys.h | build
	@echo "  CC      $<"
	$(Q)$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# The MDK's system file is third-party code: no -Werror/-Wshadow for it
build/system_nrf52833.o: $(MDK)/system_nrf52833.c build/.flags | build
	@echo "  CC      $<"
	$(Q)$(CC) $(filter-out -Werror -Wshadow,$(CFLAGS)) -c $< -o $@

build:
	mkdir -p build

build/rear_light_ant.elf: $(OBJS) linker/rear_light_ant.ld
	@echo "  LD      $@"
	$(Q)$(CC) $(LDFLAGS) $(OBJS) -lgcc -o $@

build/rear_light_ant.hex: build/rear_light_ant.elf
	@echo "  HEX     $@"
	$(Q)$(OBJCOPY) -O ihex $< $@

# The interrupt handlers the application implements must replace the weak
# defaults in the vector table (a silent name mismatch would leave an
# interrupt without a handler)
.PHONY: check-vectors
check-vectors: build/rear_light_ant.elf
	@for h in SWI2_EGU2_IRQHandler RTC1_IRQHandler GPIOTE_IRQHandler; do \
	    $(NM) $< | grep -q " T $$h$$" || { echo "missing handler $$h"; exit 1; }; \
	done

size: build/rear_light_ant.elf
	$(SIZE) $<

test: build/test_bike_light build/test_core
	build/test_bike_light
	build/test_core

HOST_CFLAGS := -Wall -Wextra -Werror -Wshadow -std=c11 -fsanitize=address,undefined -g

build/test_bike_light: test/test_bike_light.c src/bike_light.c src/bike_light.h | build
	gcc $(HOST_CFLAGS) -o $@ test/test_bike_light.c src/bike_light.c

CORE_SRCS := src/sm.c src/app_events.c src/battery_logic.c src/detectors.c src/bike_light.c

build/test_core: test/test_core.c test/test_platform.c $(CORE_SRCS) $(wildcard src/*.h) | build
	gcc $(HOST_CFLAGS) -o $@ test/test_core.c test/test_platform.c $(CORE_SRCS)

# Chip erase also clears UICR; the application hex restores NFCPINS = GPIO
flash: build/rear_light_ant.hex $(SD_HEX)
	nrfjprog --family NRF52 --program $(SD_HEX) --chiperase --verify
	nrfjprog --family NRF52 --program build/rear_light_ant.hex --sectoranduicrerase --verify
	nrfjprog --family NRF52 --reset

flash-app: build/rear_light_ant.hex
	nrfjprog --family NRF52 --program build/rear_light_ant.hex --sectoranduicrerase --verify
	nrfjprog --family NRF52 --reset

# RATE_S: measurement time of `make rate`
RATE_S ?= 30
stats:
	python3 tools/stats.py build/rear_light_ant.elf $(NM)

# TX rate and LF clock error over RATE_S seconds, CPU keeps running (J-Link)
rate:
	python3 tools/rate.py build/rear_light_ant.elf $(NM) $(RATE_S)

# bench builds: post an event, e.g. make inject EVT=BUTTON_LONG or EVT=REMOTE_SET_STATE ARG=1
inject:
	python3 tools/inject.py build/rear_light_ant.elf $(NM) $(EVT) $(ARG)

# log counter changes for WATCH_S seconds (reconnect tests)
WATCH_S ?= 300
watch:
	python3 tools/watch.py build/rear_light_ant.elf $(NM) $(WATCH_S)

clean:
	rm -rf build

-include $(OBJS:.o=.d)
