/*
 * Smart mode detectors, see detectors.h. Integer (milli) math only: the
 * nRF52 has no double precision FPU. Squared magnitudes avoid sqrt().
 */

#include "detectors.h"

#include "app_events.h"
#include "config.h"

/* Braking */
static bool braking;
static uint8_t braking_start_count;
static uint8_t braking_stop_count;
static bool baseline_valid;
static int64_t baseline_scaled; /* milli-m/s^2 * BASELINE_SCALE */
static int32_t smooth_buf[BRAKING_SMOOTHING_SAMPLES];
static uint8_t smooth_idx;
static uint8_t smooth_fill;
static int32_t smooth_sum;

/* Ambient */
static bool ambient_dark;
static uint8_t dark_sample_count;
static uint8_t bright_sample_count;

/* Stationary */
static uint16_t stationary_count;

void detectors_reset(void)
{
    braking = false;
    braking_start_count = 0U;
    braking_stop_count = 0U;

    baseline_valid = false;
    baseline_scaled = 0;

    smooth_idx = 0U;
    smooth_fill = 0U;
    smooth_sum = 0;

    ambient_dark = false;
    dark_sample_count = 0U;
    bright_sample_count = 0U;

    stationary_count = 0U;
}

/* Moving average over the last BRAKING_SMOOTHING_SAMPLES values; until the
 * window is full the average covers the samples seen so far */
static int32_t smooth(int32_t value)
{
    if (smooth_fill < BRAKING_SMOOTHING_SAMPLES) {
        smooth_buf[smooth_idx] = value;
        smooth_sum += value;
        smooth_fill++;
    } else {
        smooth_sum += value - smooth_buf[smooth_idx];
        smooth_buf[smooth_idx] = value;
    }

    smooth_idx = (uint8_t)((smooth_idx + 1U) % BRAKING_SMOOTHING_SAMPLES);

    return smooth_sum / (int32_t)smooth_fill;
}

/* Braking on baseline-corrected, smoothed Z acceleration. The baseline is
 * frozen while braking and while a start is being confirmed, so a brake
 * does not pull the baseline toward itself (SMART-8). */
static void process_braking(int64_t accel_z_milli)
{
    if (!baseline_valid) {
        baseline_scaled = accel_z_milli * BASELINE_SCALE; /* seeded by the first sample */
        baseline_valid = true;
    }

    int32_t linear_milli = (int32_t)(accel_z_milli - baseline_scaled / BASELINE_SCALE);
    int32_t smoothed_milli = smooth(linear_milli);
    bool decelerating = (smoothed_milli > BRAKING_ACCEL_THRESHOLD_MILLI);

    if (!braking) {
        if (decelerating) {
            braking_start_count++;
            if (braking_start_count >= BRAKING_START_SAMPLES) {
                braking = true;
                braking_start_count = 0U;
                braking_stop_count = 0U;
                (void)app_event_post(EVT_BRAKE_START, 0U);
            }
        } else {
            braking_start_count = 0U;
        }
    } else {
        if (!decelerating) {
            braking_stop_count++;
            if (braking_stop_count >= BRAKING_STOP_SAMPLES) {
                braking = false;
                braking_stop_count = 0U;
                (void)app_event_post(EVT_BRAKE_STOP, 0U);
            }
        } else {
            braking_stop_count = 0U;
        }
    }

    /* Slow baseline: first-order low-pass */
    if (!braking && braking_start_count == 0U) {
        int64_t target_scaled = accel_z_milli * BASELINE_SCALE;

        baseline_scaled += (target_scaled - baseline_scaled) * ACCEL_SAMPLING_INTERVAL_MS /
                           (BRAKING_BASELINE_TAU_MS + ACCEL_SAMPLING_INTERVAL_MS);
    }
}

/* Stationary: consecutive samples with |accel| near gravity. A counter of
 * consecutive samples equals "all of the last N samples", no buffer needed. */
static void process_stationary(int64_t x_milli, int64_t y_milli, int64_t z_milli)
{
    int64_t mag_sq = x_milli * x_milli + y_milli * y_milli + z_milli * z_milli;

    if (mag_sq >= STATIONARY_MIN_MAGNITUDE_SQ && mag_sq <= STATIONARY_MAX_MAGNITUDE_SQ) {
        stationary_count++;
        if (stationary_count >= STATIONARY_SAMPLES) {
            stationary_count = 0U;
            (void)app_event_post(EVT_STATIONARY_TIMEOUT, 0U);
        }
    } else {
        stationary_count = 0U;
    }
}

void detectors_accel_sample(int64_t x_milli, int64_t y_milli, int64_t z_milli)
{
    process_braking(z_milli);
    process_stationary(x_milli, y_milli, z_milli);
}

/* Ambient: one threshold, an off offset and debouncing */
void detectors_ambient_sample(int64_t millilux)
{
    if (ambient_dark) {
        if (millilux >= AMBIENT_OFF_THRESHOLD_MILLILUX) {
            bright_sample_count++;
            dark_sample_count = 0U;
            if (bright_sample_count >= AMBIENT_SAMPLES_REQUIRED) {
                ambient_dark = false;
                bright_sample_count = 0U;
                (void)app_event_post(EVT_AMBIENT_BRIGHT, 0U);
            }
        } else {
            bright_sample_count = 0U;
        }
    } else {
        if (millilux < AMBIENT_THRESHOLD_MILLILUX) {
            dark_sample_count++;
            bright_sample_count = 0U;
            if (dark_sample_count >= AMBIENT_SAMPLES_REQUIRED) {
                ambient_dark = true;
                dark_sample_count = 0U;
                (void)app_event_post(EVT_AMBIENT_DARK, 0U);
            }
        } else {
            dark_sample_count = 0U;
        }
    }
}

enum light_plan detectors_light_plan(void)
{
    /* The LED is lit at Peak while braking, and the next measurement is due
     * one light interval after braking ended (SMART-13) */
    if (braking) {
        return LIGHT_PLAN_SKIP;
    }
    if (ambient_dark) {
        /* Do not stall the accelerometer while a braking start is confirmed */
        return (braking_start_count > 0U) ? LIGHT_PLAN_SKIP : LIGHT_PLAN_BLANKED;
    }
    return LIGHT_PLAN_READ;
}

uint32_t detectors_light_next_ticks(bool measured)
{
    /* A reading that starts to end darkness is confirmed soon instead of
     * after the long blank interval */
    if (measured && ambient_dark && bright_sample_count == 0U) {
        return LIGHT_BLANK_TICKS;
    }
    return LIGHT_TICKS;
}

bool detectors_braking(void)
{
    return braking;
}

bool detectors_ambient_dark(void)
{
    return ambient_dark;
}
