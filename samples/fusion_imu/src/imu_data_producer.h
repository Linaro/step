/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef FUSION_DATA_PRODUCER_H__
#define FUSION_DATA_PRODUCER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr.h>
#include <stdlib.h>
#include <stdbool.h>
#include <step/step.h>
#include <step/measurement/measurement.h>
#include <step/measurement/ext_orientation.h>
#include <zsl/zsl.h>
#include <zsl/orientation/fusion/calibration.h>

struct imu_data_payload {
    uint32_t timestamp;
    bool streamable;

    float imu_accel_x;
    float imu_accel_y;
    float imu_accel_z;

    float imu_gyro_x;
    float imu_gyro_y;
    float imu_gyro_z;

    float imu_mag_x;
    float imu_mag_y;
    float imu_mag_z;
};

typedef void (*fusion_sensor_data_callback_t) (struct imu_data_payload *imu);

/**
 * @brief Initializes the imu data producer.
 * 
 * @param cb user callback to be executed each new imu data arrival.
 * 
 * @return int  0 on success, otherwise a negative error code.
 * 
 */
int imu_producer_start(fusion_sensor_data_callback_t cb);

/**
 * @brief Perofms the imu sensor calibration.
 */

void imu_producer_calibrate(void);

/**
 * @brief Pre-populated measurement header to copy into allocated measurements.
 */
extern const struct step_mes_header imu_measurement_header;

#ifdef __cplusplus
}
#endif
#endif