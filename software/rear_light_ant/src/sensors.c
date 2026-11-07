/*
 * Smart mode sensors, see sensors.h.
 *
 * LIS3DH (0x19): +-2 g, 10-bit normal mode, ODR from ACCEL_ODR_HZ.
 * OPT3001 (0x44): auto range, 100 ms conversion (SMART-14).
 */

#include "sensors.h"

#include "board.h"
#include "config.h"
#include "detectors.h"
#include "light.h"
#include "sd_includes.h"
#include "soft_timer.h"
#include "stats.h"
#include "twim.h"

/* LIS3DH registers */
#define LIS3DH_WHO_AM_I      0x0FU
#define LIS3DH_WHO_AM_I_ID   0x33U
#define LIS3DH_CTRL_REG1     0x20U
#define LIS3DH_CTRL_REG4     0x23U
#define LIS3DH_OUT_X_L       0x28U
#define LIS3DH_AUTO_INCREMENT 0x80U
#define LIS3DH_CTRL1_XYZ_EN  0x07U
#define LIS3DH_CTRL4_BDU     0x80U /* output registers update only after both bytes are read */

/* +-2 g in 10-bit normal mode: 4 mg per digit on the left-justified 16-bit value */
#define LIS3DH_MG_PER_DIGIT 4

/* OPT3001 registers: result 0x00, configuration 0x01 */
#define OPT3001_REG_RESULT 0x00U
#define OPT3001_REG_CONFIG 0x01U
#define OPT3001_CONFIG_CRF 0x0080U /* conversion ready flag */

static bool accel_ok;
static bool light_ok;
static bool running;

/* The last switch back to continuous conversion failed; retried on the next
 * light sample */
static bool light_config_dirty;

static uint32_t accel_phase;
static uint32_t light_wait;

enum blank_state {
    BLANK_IDLE,
    BLANK_SETTLING,   /* LED off, waiting for the PWM to apply it */
    BLANK_CONVERTING, /* single conversion running, ready after the minimum time */
    BLANK_POLLING,    /* polling the ready flag */
};

static enum blank_state blank_state;
static uint32_t blank_elapsed_ms;

/* --- LIS3DH ------------------------------------------------------------ */

static int lis3dh_write_reg(uint8_t reg, uint8_t value)
{
    uint8_t buf[2] = { reg, value };

    return twim_write(I2C_ADDR_LIS3DH, buf, sizeof(buf));
}

static int lis3dh_read_reg(uint8_t reg, uint8_t *value)
{
    return twim_write_read(I2C_ADDR_LIS3DH, &reg, 1U, value, 1U);
}

static uint8_t lis3dh_odr_bits(void)
{
    switch (ACCEL_ODR_HZ) {
    case 1U:
        return 0x1U;
    case 10U:
        return 0x2U;
    case 25U:
        return 0x3U;
    case 50U:
        return 0x4U;
    case 100U:
        return 0x5U;
    case 200U:
        return 0x6U;
    default:
        return 0x7U; /* 400 Hz */
    }
}

/* Power up with the pinned output data rate (SMART-10) */
static int lis3dh_start(void)
{
    if (lis3dh_write_reg(LIS3DH_CTRL_REG4, LIS3DH_CTRL4_BDU) != 0) {
        return -1;
    }
    return lis3dh_write_reg(LIS3DH_CTRL_REG1,
                            (uint8_t)((lis3dh_odr_bits() << 4) | LIS3DH_CTRL1_XYZ_EN));
}

static void lis3dh_stop(void)
{
    (void)lis3dh_write_reg(LIS3DH_CTRL_REG1, 0x00U); /* power-down */
}

/* @return 0 on success, values in milli-m/s^2 */
static int lis3dh_read_milli(int64_t *x, int64_t *y, int64_t *z)
{
    uint8_t reg = LIS3DH_OUT_X_L | LIS3DH_AUTO_INCREMENT;
    uint8_t raw[6];
    int16_t axis[3];

    if (twim_write_read(I2C_ADDR_LIS3DH, &reg, 1U, raw, sizeof(raw)) != 0) {
        return -1;
    }

    for (unsigned int i = 0; i < 3U; i++) {
        axis[i] = (int16_t)((uint16_t)raw[2U * i] | ((uint16_t)raw[2U * i + 1U] << 8));
    }

    /* 10-bit left-justified: shift down, 4 mg per digit, 1 g = 9.80665 m/s^2 */
    *x = ((int64_t)(axis[0] >> 6) * LIS3DH_MG_PER_DIGIT * 98067) / 10000;
    *y = ((int64_t)(axis[1] >> 6) * LIS3DH_MG_PER_DIGIT * 98067) / 10000;
    *z = ((int64_t)(axis[2] >> 6) * LIS3DH_MG_PER_DIGIT * 98067) / 10000;
    return 0;
}

/* --- OPT3001 ----------------------------------------------------------- */

static int opt3001_config_write(uint16_t config)
{
    uint8_t buf[3] = { OPT3001_REG_CONFIG, (uint8_t)(config >> 8), (uint8_t)(config & 0xFFU) };

    return twim_write(I2C_ADDR_OPT3001, buf, sizeof(buf));
}

static int opt3001_config_read(uint16_t *config)
{
    uint8_t reg = OPT3001_REG_CONFIG;
    uint8_t buf[2];

    if (twim_write_read(I2C_ADDR_OPT3001, &reg, 1U, buf, sizeof(buf)) != 0) {
        return -1;
    }
    *config = (uint16_t)(((uint16_t)buf[0] << 8) | buf[1]);
    return 0;
}

static int opt3001_read_millilux(int64_t *millilux)
{
    uint8_t reg = OPT3001_REG_RESULT;
    uint8_t buf[2];
    uint16_t result;
    uint32_t exponent;
    uint32_t mantissa;

    if (twim_write_read(I2C_ADDR_OPT3001, &reg, 1U, buf, sizeof(buf)) != 0) {
        return -1;
    }
    result = (uint16_t)(((uint16_t)buf[0] << 8) | buf[1]);
    exponent = result >> 12;
    mantissa = result & 0x0FFFU;
    if (exponent == 0x0FU) {
        return -1; /* reserved */
    }

    *millilux = (int64_t)(mantissa << exponent) * 10; /* 0.01 lux per LSB */
    return 0;
}

/* --- Light measurement ------------------------------------------------- */

/*
 * Read the last continuous conversion (LED not lit). Reading the
 * configuration register clears the conversion-ready flag, so a set flag
 * means a new conversion finished since the previous read; this also
 * rejects the all-zero result register right after power-up.
 *
 * @return 0 on success, 1 if no new conversion is ready, -1 on an error
 */
static int read_light(int64_t *millilux)
{
    uint16_t config;

    if (opt3001_config_read(&config) != 0) {
        return -1;
    }
    if ((config & OPT3001_CONFIG_CRF) == 0U) {
        return 1;
    }
    return (opt3001_read_millilux(millilux) == 0) ? 0 : -1;
}

/* A blank always ends here, also on errors (SMART-15) */
static void blank_finish(bool ok, int64_t millilux)
{
    soft_timer_stop(ST_BLANK);
    blank_state = BLANK_IDLE;
    light_blank_end();

    if (opt3001_config_write(OPT3001_CONFIG_CONTINUOUS) != 0) {
        light_config_dirty = true;
    }

    if (ok) {
        proto_stats.millilux = (int32_t)millilux;
        detectors_ambient_sample(millilux);
    }
    proto_stats.blank_count++;
    light_wait = detectors_light_next_ticks(ok);
}

static void blank_begin(void)
{
    light_blank_begin();
    blank_state = BLANK_SETTLING;
    soft_timer_start(ST_BLANK, AMBIENT_BLANK_SETTLE_MS, 0U);
}

void sensors_on_blank_step(void)
{
    uint16_t config = 0U;
    int64_t millilux = 0;

    switch (blank_state) {
    case BLANK_SETTLING:
        /* Writing the configuration starts the conversion at once, so LED
         * and conversion are synchronised */
        if (opt3001_config_write(OPT3001_CONFIG_SINGLE_SHOT) != 0) {
            blank_finish(false, 0);
            return;
        }
        blank_state = BLANK_CONVERTING;
        blank_elapsed_ms = OPT3001_CONVERSION_MIN_MS;
        soft_timer_start(ST_BLANK, OPT3001_CONVERSION_MIN_MS, 0U);
        return;
    case BLANK_CONVERTING:
    case BLANK_POLLING:
        if (opt3001_config_read(&config) != 0) {
            blank_finish(false, 0);
            return;
        }
        if ((config & OPT3001_CONFIG_CRF) != 0U) {
            bool ok = (opt3001_read_millilux(&millilux) == 0);

            blank_finish(ok, millilux);
            return;
        }
        if (blank_elapsed_ms >= AMBIENT_BLANK_TIMEOUT_MS) {
            blank_finish(false, 0);
            return;
        }
        blank_state = BLANK_POLLING;
        blank_elapsed_ms += AMBIENT_BLANK_POLL_MS;
        soft_timer_start(ST_BLANK, AMBIENT_BLANK_POLL_MS, 0U);
        return;
    case BLANK_IDLE:
    default:
        return; /* stale step */
    }
}

/* Take one ambient measurement and decide when the next one is due */
static void sample_light(void)
{
    int64_t millilux = 0;
    int ret;

    if (!light_ok) {
        light_wait = LIGHT_TICKS;
        return;
    }

    if (light_config_dirty) {
        if (opt3001_config_write(OPT3001_CONFIG_CONTINUOUS) == 0) {
            light_config_dirty = false;
        }
        light_wait = LIGHT_TICKS;
        return;
    }

    switch (detectors_light_plan()) {
    case LIGHT_PLAN_BLANKED:
        blank_begin(); /* its end sets the next due time */
        return;
    case LIGHT_PLAN_READ:
        ret = read_light(&millilux);
        if (ret == 0) {
            proto_stats.millilux = (int32_t)millilux;
            detectors_ambient_sample(millilux);
        }
        light_wait = detectors_light_next_ticks(ret == 0);
        return;
    case LIGHT_PLAN_SKIP:
    default:
        light_wait = detectors_light_next_ticks(false);
        return;
    }
}

static void sample_accel(void)
{
    int64_t x;
    int64_t y;
    int64_t z;

    if (!accel_ok) {
        return;
    }
    /* A failed read neither counts nor resets the stationary count (SMART-5) */
    if (lis3dh_read_milli(&x, &y, &z) == 0) {
        proto_stats.accel_milli[0] = (int32_t)x;
        proto_stats.accel_milli[1] = (int32_t)y;
        proto_stats.accel_milli[2] = (int32_t)z;
        detectors_accel_sample(x, y, z);
    }
}

/* --- Public interface -------------------------------------------------- */

int sensors_init(void)
{
    uint8_t id = 0U;

    twim_enable();

    accel_ok = (lis3dh_read_reg(LIS3DH_WHO_AM_I, &id) == 0 && id == LIS3DH_WHO_AM_I_ID);
    light_ok = (opt3001_config_write(OPT3001_CONFIG_CONTINUOUS) == 0);

    /* Without the 100 ms conversion time a blanked measurement cannot work
     * (SMART-14): the light sensor counts as failed */
    if (accel_ok) {
        lis3dh_stop();
    }
    if (light_ok) {
        (void)opt3001_config_write(OPT3001_CONFIG_SHUTDOWN);
    }
    twim_disable();

    proto_stats.accel_ok = accel_ok ? 1U : 0U;
    proto_stats.light_ok = light_ok ? 1U : 0U;
    return (accel_ok || light_ok) ? 0 : -1;
}

void sensors_start(void)
{
    if (running) {
        return;
    }

    twim_enable();
    detectors_reset();
    blank_state = BLANK_IDLE;
    light_config_dirty = false;

    if (accel_ok && lis3dh_start() != 0) {
        accel_ok = false; /* SMART-7: continue with the other sensor */
    }
    if (light_ok && opt3001_config_write(OPT3001_CONFIG_CONTINUOUS) != 0) {
        light_ok = false;
    }
    proto_stats.accel_ok = accel_ok ? 1U : 0U;
    proto_stats.light_ok = light_ok ? 1U : 0U;

    /* Both sensors are sampled right away, then each on its own schedule */
    accel_phase = 0U;
    light_wait = 0U;
    running = true;
    soft_timer_start(ST_SENSOR, SENSOR_TICK_MS, SENSOR_TICK_MS);
}

void sensors_stop(void)
{
    if (!running) {
        return;
    }

    running = false;
    soft_timer_stop(ST_SENSOR);

    if (blank_state != BLANK_IDLE) {
        soft_timer_stop(ST_BLANK);
        blank_state = BLANK_IDLE;
        light_blank_end(); /* a blank always ends (SMART-15) */
    }

    if (accel_ok) {
        lis3dh_stop();
    }
    if (light_ok) {
        (void)opt3001_config_write(OPT3001_CONFIG_SHUTDOWN);
    }
    twim_disable();
}

void sensors_on_tick(void)
{
    if (!running) {
        return; /* stale tick */
    }

    if (accel_phase == 0U) {
        sample_accel();
    }
    accel_phase = (accel_phase + 1U) % ACCEL_TICKS;

    if (blank_state != BLANK_IDLE) {
        return; /* a measurement is running */
    }
    if (light_wait == 0U) {
        sample_light();
    } else {
        light_wait--;
    }
}
