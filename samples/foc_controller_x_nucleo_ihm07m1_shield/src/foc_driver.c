/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <arm_math.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <step/sample_pool.h>
#include <step/proc_mgr.h>
#include "foc_driver.h"

/** AS5600 I2C Encoder constants */
#define AS5600_SLAVE_ADDR 0x36
#define AS5600_ANGLE_REGISTER_H 0x0E
#define AS5600_PULSES_PER_REVOLUTION 4096.0f
#define AS5600_READING_MASK 0xFFF 

#define ROTOR_DRV_PAYLOAD_SZ      (sizeof(struct foc_controller_payload))
#define ROTOR_DRV_PAYLOAD_SRC_ID  (2)

/* PWM frequency about 16KHz */
#define PWM_PERIOD_NSEC 62500

/**
 * @brief Pre-populated measurement header to copy into allocated measurements.
 */
static struct step_mes_header rotor_sensor_header = {
	/* Filter word. */
	.filter = {
		.base_type = STEP_MES_TYPE_PHASE,
		.flags = {
			.data_format = STEP_MES_FORMAT_NONE,
			.encoding = STEP_MES_ENCODING_NONE,
			.compression = STEP_MES_COMPRESSION_NONE,
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
		.len = ROTOR_DRV_PAYLOAD_SZ,
		.fragment = 0,
		.sourceid = ROTOR_DRV_PAYLOAD_SRC_ID,
	}
};

static void foc_driver_rotor_position_sample_thread(void *arg);

/* Rotor sensor measurement thread. */
K_THREAD_DEFINE(rotor_sample_tid, 4096,
		foc_driver_rotor_position_sample_thread, NULL, NULL, NULL,
		0, 0, 500);

const static float encoder_to_degrees_ratio = (360.0f) / AS5600_PULSES_PER_REVOLUTION;

static foc_measurement_callback_t user_callback;
static float pole_pairs;

static struct {
	uint16_t raw;
	uint16_t zero_offset;
} encoder_reading;

static const struct device *inverter_pwm = DEVICE_DT_GET(DT_NODELABEL(inverter_pwm));
static const struct device *encoder_i2c = DEVICE_DT_GET(DT_NODELABEL(i2c1));

static struct gpio_dt_spec enable_u = GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(inverter_enable),gpios,0);
static struct gpio_dt_spec enable_v = GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(inverter_enable),gpios,1);
static struct gpio_dt_spec enable_w = GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(inverter_enable),gpios,2);

static uint16_t foc_driver_read_encoder(void)
{
	uint16_t raw;
	uint8_t read_data[2] = {0,0};
	uint8_t angle_reg = AS5600_ANGLE_REGISTER_H;

	/* gets the raw position from encoder */
	int err = i2c_write_read(encoder_i2c, 
						AS5600_SLAVE_ADDR,
						&angle_reg,
						1,
						&read_data,
						sizeof(read_data));

	if(err) {
		return 0xFFFF;
	}

	raw = ((uint16_t)read_data[0] << 8) | read_data[1];

	/* wrap to its maximum value */
	if(raw > AS5600_PULSES_PER_REVOLUTION - 1) {
		raw = AS5600_PULSES_PER_REVOLUTION - 1 ;
	}

	return raw;
}

static void foc_driver_get_rotor_position(struct step_measurement * rotor_measurement)
{
	struct foc_controller_payload *p = rotor_measurement->payload;
	struct foc_feeback_sensor_data *fdbk = &p->state;

	fdbk->motor_pole_pairs = pole_pairs;

	/* update the encoder reading */
	uint16_t counts = foc_driver_read_encoder();

	/* keeps a ZOH (Zero Order Hold) by just accepting valid measurements to 
	 * reduce position reading distortion
	 */
	if(counts <= (AS5600_PULSES_PER_REVOLUTION - 1)) {
		/* valid readings, update the encoder */
		encoder_reading.raw = counts;
	}

	/* make encoder measures to radians for mech and electrical angle */
	fdbk->rotor_position = (float)((encoder_reading.raw - encoder_reading.zero_offset) &
						 AS5600_READING_MASK) * encoder_to_degrees_ratio;
	fdbk->e_rotor_position = fdbk->motor_pole_pairs * fdbk->rotor_position;

	p->timestamp = k_uptime_get_32();
}

static void foc_driver_rotor_position_sample_thread(void *arg)
{	
	int mcnt;

	for(;;) {
		struct step_measurement *rotor_measurement =  step_sp_alloc(ROTOR_DRV_PAYLOAD_SZ);

		if(rotor_measurement == NULL) {
			k_sleep(K_MSEC(1));
			continue;
		}
		
		foc_driver_get_rotor_position(rotor_measurement);

		if(user_callback) {
			user_callback(rotor_measurement);
		}

		/* fill measurements information before publishing: */
		rotor_measurement->header.filter_bits = rotor_sensor_header.filter_bits;
		rotor_measurement->header.unit_bits = rotor_sensor_header.unit_bits;
		rotor_measurement->header.srclen_bits = rotor_sensor_header.srclen_bits;

		/* publish sensor measurement to be processed by the waiters
		* in that case, FoC pipeline would be one of the waiters 
		*/
		step_sp_put(rotor_measurement);
		step_pm_poll(&mcnt, true);
	}
}

int foc_driver_initialize(float motor_pole_pairs, foc_measurement_callback_t cb)
{
	int rc = 0;

	user_callback = cb;
	pole_pairs = motor_pole_pairs;

	encoder_reading.raw = 0;
	encoder_reading.zero_offset = 0;

	i2c_configure(encoder_i2c, I2C_MODE_MASTER | I2C_SPEED_FAST);
	foc_driver_set_duty_cycle(0.0f, 0.0f, 0.0f);

	gpio_pin_configure_dt(&enable_u, GPIO_OUTPUT);
	gpio_pin_configure_dt(&enable_v, GPIO_OUTPUT);
	gpio_pin_configure_dt(&enable_w, GPIO_OUTPUT);

	gpio_pin_set_dt(&enable_u, 1);
	gpio_pin_set_dt(&enable_v, 1);
	gpio_pin_set_dt(&enable_w, 1);

	return rc;
}

int foc_driver_set_encoder_offset(void)
{
	/* align rotor each startup */
	encoder_reading.zero_offset = foc_driver_read_encoder() & AS5600_READING_MASK;

	return 0;
}

void foc_driver_set_duty_cycle(float dc_u, float dc_v, float dc_w)
{
	/* saturate the duties */
	if(dc_u > 1.0f) {
		dc_u = 1.0f;
	} else if (dc_u < 0.0f) {
		dc_u = 0.0f;
	}

	if(dc_v > 1.0f) {
		dc_v = 1.0f;
	} else if (dc_v < 0.0f) {
		dc_v = 0.0f;
	}

	if(dc_w > 1.0f) {
		dc_w = 1.0f;
	} else if (dc_w < 0.0f) {
		dc_w = 0.0f;
	}

	/* scale the duty cycle to nanoseconds */
	dc_u *= PWM_PERIOD_NSEC - 1;
	dc_v *= PWM_PERIOD_NSEC - 1;
	dc_w *= PWM_PERIOD_NSEC - 1;

	pwm_set(inverter_pwm, 1 , PWM_PERIOD_NSEC, dc_u, 0);
	pwm_set(inverter_pwm, 2 , PWM_PERIOD_NSEC, dc_v, 0);
	pwm_set(inverter_pwm, 3 , PWM_PERIOD_NSEC, dc_w, 0);
}
