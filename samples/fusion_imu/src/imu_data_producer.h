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

struct imu_data_payload {
    uint32_t timestamp;

    float imu_accel_x;
    float imu_accel_y;
    float imu_accel_z;

    float imu_gyro_x;
    float imu_gyro_y;
    float imu_gyro_z;

    float imu_roll;
    float imu_pitch;
    float imu_yaw;
};

typedef void (*fusion_sensor_data_callback_t) (struct imu_data_payload *imu);

int imu_producer_start(fusion_sensor_data_callback_t cb);

#ifdef __cplusplus
}
#endif