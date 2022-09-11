/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include "imu_data_producer.h"
#include "zsl_fusion_config.h"

LOG_MODULE_REGISTER(imu_data_producer);

static const struct device *imu_dev = DEVICE_DT_GET(DT_NODELABEL(board_imu));
static const struct device *mag_dev = DEVICE_DT_GET(DT_NODELABEL(board_mag));
static bool imu_calibrated = false;
static fusion_sensor_data_callback_t imu_user_callback = NULL;
ZSL_VECTOR_DEF(gyro_offset_correcton, 3);

/* create the imu measurement header */
const struct step_mes_header imu_measurement_header = {
	/* Filter word. */
	.filter = {
		.base_type = STEP_MES_TYPE_ORIENTATION,
		.flags = {
			.timestamp = STEP_MES_TIMESTAMP_UPTIME_MS_32,
		},
	},
	/* SI Unit word. */
	.unit = {
		.si_unit = STEP_MES_UNIT_SI_RADIAN,
		.ctype = STEP_MES_UNIT_CTYPE_IEEE754_FLOAT32,
		.scale_factor = STEP_MES_SI_SCALE_NONE,
	},
	/* Source/Len word. */
	.srclen = {
		.len = sizeof(struct imu_data_payload),
		.vec_sz = 3,
		.sourceid = 2,
	}
};

/* imu measurement thread. */
static void imu_sample_thread(void *arg);
K_THREAD_DEFINE(imu_sample_tid, 8192, imu_sample_thread, NULL, NULL, NULL, 1, 0, 0);

static inline float imu_zephyr_sensor_to_float(struct sensor_value *val)
{
	return (float)val->val1 + (float)val->val2 / 1000000;
}

static void imu_fetch_data(struct imu_data_payload *imu)
{
	struct sensor_value x, y, z;

	/* float data is more convenient to deal with fusion
		* processing, so after fetching, map the sensor data
		* to float
		*/
	sensor_sample_fetch_chan(imu_dev, SENSOR_CHAN_ACCEL_XYZ);
	sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_X, &x);
	sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_Y, &y);
	sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_Z, &z);

	imu->imu_accel_x = imu_zephyr_sensor_to_float(&x);
	imu->imu_accel_y = imu_zephyr_sensor_to_float(&y);
	imu->imu_accel_z = imu_zephyr_sensor_to_float(&z);

	sensor_sample_fetch_chan(imu_dev, SENSOR_CHAN_GYRO_XYZ);
	sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_X, &x);
	sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_Y, &y);
	sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_Z, &z);

	imu->imu_gyro_x = imu_zephyr_sensor_to_float(&x);
	imu->imu_gyro_y = imu_zephyr_sensor_to_float(&y);
	imu->imu_gyro_z = imu_zephyr_sensor_to_float(&z);

	sensor_sample_fetch_chan(mag_dev, SENSOR_CHAN_MAGN_XYZ);
	sensor_channel_get(mag_dev, SENSOR_CHAN_MAGN_X, &x);
	sensor_channel_get(mag_dev, SENSOR_CHAN_MAGN_Y, &y);
	sensor_channel_get(mag_dev, SENSOR_CHAN_MAGN_Z, &z);

	imu->imu_mag_x = imu_zephyr_sensor_to_float(&x) * 0.01;
	imu->imu_mag_y = imu_zephyr_sensor_to_float(&y) * 0.01;
	imu->imu_mag_z = imu_zephyr_sensor_to_float(&z) * 0.01;

	imu->timestamp = k_uptime_get_32();

	/* mag plus gyro were calibrated, so correct them */
	if(imu_calibrated) {
		ZSL_VECTOR_DEF(gyro_sample, 3);

		/* correct the gyro samples */
		gyro_sample.data[0] = imu->imu_gyro_x;
		gyro_sample.data[1] = imu->imu_gyro_y;
		gyro_sample.data[2] = imu->imu_gyro_z;

		gyro_sample.data[0] -= gyro_offset_correcton.data[0];
		gyro_sample.data[1] -= gyro_offset_correcton.data[1];
		gyro_sample.data[2] -= gyro_offset_correcton.data[2];

		imu->imu_gyro_x = gyro_sample.data[0];
		imu->imu_gyro_y = gyro_sample.data[1];
		imu->imu_gyro_z = gyro_sample.data[2];

	}
}

static void imu_set_data_rate(void) 
{
    int rc = 0;
	struct sensor_value odr_attr;

    /* sets the output data rate to 100Hz */
	odr_attr.val1 = 104;
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

    /* sets the output data rate to 100Hz */
	odr_attr.val1 = 100;
	odr_attr.val2 = 0;

	rc = sensor_attr_set(mag_dev, SENSOR_CHAN_MAGN_XYZ,
			SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr);
	if (rc != 0) {
		printk("Cannot set sampling frequency for gyro.\n");
	}
}

static void imu_sample_thread(void *arg) 
{
    struct imu_data_payload measurement;

    imu_set_data_rate();
	imu_producer_calibrate();

    for(;;) {
        imu_fetch_data(&measurement);
        
        /* defer the measured data to the application callback*/
        if(imu_user_callback) {
            imu_user_callback(&measurement);
        }

		k_sleep(K_MSEC(10));
    }
}

int imu_producer_start(fusion_sensor_data_callback_t cb)
{
	while(!imu_calibrated) {
		k_sleep(K_MSEC(1));
	}

    imu_user_callback = cb;
    return 0;
}

void imu_producer_calibrate(void)
{
	ZSL_VECTOR_DEF(gyro_sample, 3);
	struct imu_data_payload imu;

	imu_calibrated = false;

	LOG_INF("Calibrating gyro, please place the board in flat surface and wait up 5 seconds!");
	k_sleep(K_MSEC(2));

	zsl_vec_init(&gyro_sample);

	/* takes up 2 seconds of gyro samples in standstill */
	for(int i = 0; i < 200; i++) {

		imu_fetch_data(&imu);
		gyro_sample.data[0] += imu.imu_gyro_x;
		gyro_sample.data[1] += imu.imu_gyro_y;
		gyro_sample.data[2] += imu.imu_gyro_z;
		k_sleep(K_MSEC(10));
	}

	/* compute the mean of each component */
	zsl_vec_scalar_div(&gyro_sample, 200.);

	/* sets the offset correction vector */
	zsl_vec_copy(&gyro_offset_correcton, &gyro_sample);

	LOG_INF("Gyro offset correction vector: %f, %f, %f",
		gyro_sample.data[0],
		gyro_sample.data[1],
		gyro_sample.data[2]);

	fusion_driver.init_handler(104, fusion_driver.config);

	imu_calibrated = true;
}
