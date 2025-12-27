#ifndef SENSOR_DATA_COLLECTOR_H
#define SENSOR_DATA_COLLECTOR_H

typedef struct {
    struct sensor_value temp; // internal nRF52833 temperature
    struct sensor_value light; // OPT3001 ambient light intensity
    struct sensor_value accel_x; // LIS3DH accelerometer X-axis
    struct sensor_value accel_y; // LIS3DH accelerometer Y-axis
    struct sensor_value accel_z; // LIS3DH accelerometer Z-axis
} sensor_readings_t;

#endif /* SENSOR_DATA_COLLECTOR_H */