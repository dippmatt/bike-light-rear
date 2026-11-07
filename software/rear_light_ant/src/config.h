/*
 * Tunable constants of the rear light firmware. The requirements in SPEC.md
 * name these constants in [...]: this file is the single source of truth.
 * Pin assignments are in board.h, protocol constants of the ANT+ pages in
 * bike_light.c.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

/* --- System ------------------------------------------------------------ */

#define APP_EVENT_QUEUE_DEPTH 16U

/* --- Light output ------------------------------------------------------ */

#define LIGHT_PWM_PERIOD_USEC 1000U
#define LIGHT_PULSE_OFF  0U
#define LIGHT_PULSE_BASE 200U /* normal riding brightness, 20 % duty */
#define LIGHT_PULSE_PEAK 800U /* brake / flash peak brightness */

/* Flash: double flash every cycle (peak, base, peak, base for the rest) */
#define FLASH_CYCLE_MS 1500U
#define FLASH_STEP_MS  70U
#define FLASH_REST_MS  (FLASH_CYCLE_MS - 3U * FLASH_STEP_MS)

/* --- Button ------------------------------------------------------------ */

#define DEBOUNCE_MS        200U /* minimum time between accepted presses */
#define SAMPLE_INTERVAL_MS 100U /* press-length sampling interval */
#define LONG_PRESS_SAMPLES 10U  /* held for 10 samples (1 s) -> long press */

/* Selection window: short presses cycle the Active Modes for this many
 * battery ticks after the light is switched on from an inactive state */
#define MODE_SELECT_WINDOW_BATTERY_TICKS 4U

/* Deep Sleep entry waits for a stable button release */
#define SLEEP_RELEASE_WAIT_MS      10000U
#define SLEEP_RELEASE_POLL_MS      20U
#define SLEEP_RELEASE_STABLE_POLLS 3U

/* --- Status LED -------------------------------------------------------- */

#define BLINK_INTERVAL_LOW_BATTERY_MS 500U
#define BLINK_INTERVAL_CHARGING_MS    300U

/* Heartbeat while awake and Off (reachable by the controller): a short pulse
 * every period, so Off can be told from Deep Sleep without a solid LED */
#define HEARTBEAT_PERIOD_MS 2000U
#define HEARTBEAT_ON_MS     100U

/* --- USB and battery --------------------------------------------------- */

#define USB_POLL_INTERVAL_MS 1000U

#define BATTERY_SAMPLE_INTERVAL_MS 5000U
#define BATTERY_CRITICAL_MV        3000U
#define BATTERY_LOW_MV             3400U
#define BATTERY_CONSECUTIVE        3U

/* ANT+ battery status bands (page 1) and percentage range (page 19) */
#define BATTERY_STATUS_OK_MV   3600U
#define BATTERY_STATUS_GOOD_MV 3900U
#define BATTERY_EMPTY_MV       3000U
#define BATTERY_FULL_MV        4200U

/* --- ANT+ ---------------------------------------------------------------- */

#define ANT_DEVICE_TYPE_BIKE_LIGHT 0x23U
#define ANT_RF_FREQUENCY           57U   /* 2457 MHz */
#define ANT_CHANNEL_PERIOD         4084U /* 32768 Hz counts, ~8.02 Hz */

/* Controller requests are ignored this long after a local state change */
#define REMOTE_STABLE_WINDOW_MS 3000U

/* A connected light returns to unconnected after this many battery ticks
 * (2 minutes) without any message from a controller */
#define ANT_SILENCE_TIMEOUT_BATTERY_TICKS 24U

/* --- Smart sensors --------------------------------------------------------
 *
 * Sampling rate and detection timing are decoupled: every detector is
 * specified in milliseconds and converted to a sample count at compile time
 * (rounded up, so a time is never shorter than specified).
 */

#define MS_TO_SAMPLES(ms, interval_ms) (((ms) + (interval_ms) - 1U) / (interval_ms))

/* The accelerometer and the light sensor are sampled on their own schedule */
#define ACCEL_SAMPLING_INTERVAL_MS 50U
#define LIGHT_SAMPLING_INTERVAL_MS 500U

/* Timer period of the sensor module: the greatest common divisor of both */
#define SENSOR_TICK_MS 50U

#define ACCEL_TICKS (ACCEL_SAMPLING_INTERVAL_MS / SENSOR_TICK_MS)
#define LIGHT_TICKS (LIGHT_SAMPLING_INTERVAL_MS / SENSOR_TICK_MS)

_Static_assert(ACCEL_SAMPLING_INTERVAL_MS % SENSOR_TICK_MS == 0U,
               "accelerometer interval must be a multiple of the sensor tick");
_Static_assert(LIGHT_SAMPLING_INTERVAL_MS % SENSOR_TICK_MS == 0U,
               "light sensor interval must be a multiple of the sensor tick");

/* Accelerometer output data rate (Hz), pinned at init. At least the polling
 * rate so every poll reads a fresh value. LIS3DH: 1, 10, 25, 50, 100, 200, 400 */
#define ACCEL_ODR_HZ 25U

_Static_assert(ACCEL_ODR_HZ * ACCEL_SAMPLING_INTERVAL_MS >= 1000U,
               "accelerometer output rate must not be below the polling rate");

/* Braking: filtered Z acceleration (milli-m/s^2, +5.0 m/s^2) */
#define BRAKING_ACCEL_THRESHOLD_MILLI 5000
#define BRAKING_START_CONFIRM_MS      100U
#define BRAKING_STOP_CONFIRM_MS       150U
#define BRAKING_HOLD_MS               500U
#define BRAKING_SMOOTHING_MS          150U
#define BRAKING_BASELINE_TAU_MS       8000U

#define BRAKING_START_SAMPLES \
    MS_TO_SAMPLES(BRAKING_START_CONFIRM_MS, ACCEL_SAMPLING_INTERVAL_MS)
#define BRAKING_STOP_SAMPLES \
    MS_TO_SAMPLES(BRAKING_STOP_CONFIRM_MS + BRAKING_HOLD_MS, ACCEL_SAMPLING_INTERVAL_MS)
#define BRAKING_SMOOTHING_SAMPLES \
    MS_TO_SAMPLES(BRAKING_SMOOTHING_MS, ACCEL_SAMPLING_INTERVAL_MS)

_Static_assert(BRAKING_START_SAMPLES >= 1U && BRAKING_START_SAMPLES <= UINT8_MAX,
               "braking start confirm out of range");
_Static_assert(BRAKING_STOP_SAMPLES >= 1U && BRAKING_STOP_SAMPLES <= UINT8_MAX,
               "braking stop confirm plus hold out of range");
_Static_assert(BRAKING_SMOOTHING_SAMPLES >= 1U && BRAKING_SMOOTHING_SAMPLES <= UINT8_MAX,
               "braking smoothing window out of range");

/* The baseline is kept with extra fractional resolution */
#define BASELINE_SCALE 1024

/* Ambient light (milli-lux): dark below the threshold, bright again at or
 * above threshold + offset */
#define AMBIENT_THRESHOLD_MILLILUX  30000
#define AMBIENT_OFF_OFFSET_MILLILUX 10000
#define AMBIENT_OFF_THRESHOLD_MILLILUX \
    (AMBIENT_THRESHOLD_MILLILUX + AMBIENT_OFF_OFFSET_MILLILUX)
#define AMBIENT_SAMPLES_REQUIRED    2U

/* While the LED is lit at Base, a blanked measurement is taken this often */
#define LIGHT_BLANK_INTERVAL_MS 30000U
#define LIGHT_BLANK_TICKS       MS_TO_SAMPLES(LIGHT_BLANK_INTERVAL_MS, SENSOR_TICK_MS)

/* OPT3001: 100 ms conversion plus up to 10 ms for the first auto-range step.
 * The ready flag is polled after the minimum time. */
#define OPT3001_CONVERSION_MIN_MS 90U
#define AMBIENT_BLANK_SETTLE_MS   2U   /* PWM applies the new duty at the next 1 ms period */
#define AMBIENT_BLANK_POLL_MS     5U
#define AMBIENT_BLANK_TIMEOUT_MS  250U /* from the start of the conversion */

/* OPT3001 configuration words: RN=1100 (auto range), CT=0 (100 ms),
 * M=11 continuous / 01 single-shot / 00 shutdown, L=1 (power-up default) */
#define OPT3001_CONFIG_CONTINUOUS  0xC610U
#define OPT3001_CONFIG_SINGLE_SHOT 0xC210U
#define OPT3001_CONFIG_SHUTDOWN    0xC010U

/* Stationary (auto-off): acceleration magnitude (milli-m/s^2) near 1 g with
 * a 100 % tolerance (0 .. 2 g); squared values avoid a square root */
#define STATIONARY_MIN_MAGNITUDE_SQ ((int64_t)0)
#define STATIONARY_MAX_MAGNITUDE_SQ ((int64_t)19620 * 19620)
#define STATIONARY_TIMEOUT_MS       150000U /* 2.5 minutes */
#define STATIONARY_SAMPLES \
    MS_TO_SAMPLES(STATIONARY_TIMEOUT_MS, ACCEL_SAMPLING_INTERVAL_MS)

_Static_assert(STATIONARY_SAMPLES >= 1U && STATIONARY_SAMPLES <= UINT16_MAX,
               "stationary timeout does not fit the sample counter");

/* --- Smart parked (SMART-16..19) -------------------------------------------
 *
 * After the stationary timeout Smart parks: only the LIS3DH's own motion
 * detector runs (low-power 10 Hz, high-pass filtered, latched flag). Its INT
 * pins are not wired, so the flag is polled.
 */

#define PARKED_POLL_INTERVAL_MS 1000U

/* High-pass filtered acceleration on any axis above this wakes the light.
 * INT1_THS has 16 mg per LSB at +-2 g, 7 bits. */
#define MOTION_WAKE_THRESHOLD_MG 300U /* above wind and brushing, below rolling vibration */
#define MOTION_WAKE_THS          MS_TO_SAMPLES(MOTION_WAKE_THRESHOLD_MG, 16U)
#define MOTION_WAKE_DURATION     0U /* samples (1/ODR) above threshold, 0 = one sample */

_Static_assert(MOTION_WAKE_THS >= 1U && MOTION_WAKE_THS <= 127U,
               "motion wake threshold out of range");

/* Status LED while parked: a very short pulse, like a smoke detector */
#define PARKED_PULSE_ON_MS     30U
#define PARKED_PULSE_PERIOD_MS 5000U

#endif /* CONFIG_H */
