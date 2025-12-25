#ifndef SENSOR_DATA_COLLECTOR_H
#define SENSOR_DATA_COLLECTOR_H

typedef struct {
    struct sensor_value temp; // internal nRF52833 temperature
    struct sensor_value light; // OPT3001 ambient light intensity
    struct sensor_value accel; // LIS3DH accelerometer data
} sensor_readings_t;

#endif /* SENSOR_DATA_COLLECTOR_H */