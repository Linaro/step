/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include "imu_data_producer.h"

static const struct device *imu_dev = DEVICE_DT_GET(DT_NODELABEL(board_imu));
static fusion_sensor_data_callback_t imu_user_callback;

/* imu measurement thread. */
static void imu_sample_thread(void *arg);
K_THREAD_DEFINE(imu_sample_tid, 4096, imu_sample_thread, NULL, NULL, NULL, -1, 0, 0);

static inline float imu_zephyr_sensor_to_float(struct sensor_value *val)
{
	return (val->val1 + (float)val->val2 * 0.000001f);
}

static void imu_fetch_data(const struct device *dev, struct imu_data_payload *imu)
{
	struct sensor_value x, y, z;

    /* float data is more convenient to deal with fusion
     * processing, so after fetching, map the sensor data
     * to float
     */
	sensor_sample_fetch_chan(dev, SENSOR_CHAN_ACCEL_XYZ);
	sensor_channel_get(dev, SENSOR_CHAN_ACCEL_X, &x);
	sensor_channel_get(dev, SENSOR_CHAN_ACCEL_Y, &y);
	sensor_channel_get(dev, SENSOR_CHAN_ACCEL_Z, &z);

    imu->imu_accel_x = imu_zephyr_sensor_to_float(&x);
    imu->imu_accel_y = imu_zephyr_sensor_to_float(&y);
    imu->imu_accel_z = imu_zephyr_sensor_to_float(&z);

	sensor_sample_fetch_chan(dev, SENSOR_CHAN_GYRO_XYZ);
	sensor_channel_get(dev, SENSOR_CHAN_GYRO_X, &x);
	sensor_channel_get(dev, SENSOR_CHAN_GYRO_Y, &y);
	sensor_channel_get(dev, SENSOR_CHAN_GYRO_Z, &z);

    imu->imu_gyro_x = imu_zephyr_sensor_to_float(&x);
    imu->imu_gyro_y = imu_zephyr_sensor_to_float(&y);
    imu->imu_gyro_z = imu_zephyr_sensor_to_float(&z);
}

static void imu_set_data_rate(void) 
{
    int rc = 0;
	struct sensor_value odr_attr;

    /* sets the output data rate to 100Hz */
	odr_attr.val1 = 100;
	odr_attr.val2 = 0;

	rc = sensor_attr_set(imu_dev, SENSOR_CHAN_ACCEL_XYZ,
			SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr);
	if (rc != 0) {
		printk("Cannot set sampling frequency for accelerometer.\n");
	}

	rc = sensor_attr_set(imu_dev, SENSOR_CHAN_GYRO_XYZ,
			SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr);
	if (rc != 0) {
		printk("Cannot set sampling frequency for gyro.\n");
	}
}

static void imu_sample_thread(void *arg) 
{
    struct imu_data_payload measurement;

    imu_set_data_rate();

    for(;;) {
        imu_fetch_data(imu_dev, &measurement);
        
        /* defer the measured data to the application callback*/
        if(imu_user_callback) {
            imu_user_callback(&measurement);
        }

        /* reads the IMU data at 2x the ODR */
        k_sleep(K_MSEC(5));
    }
}

int imu_producer_start(fusion_sensor_data_callback_t cb)
{
    imu_user_callback = cb;
    return 0;
}
