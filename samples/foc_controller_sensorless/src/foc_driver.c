/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <arm_math.h>
#include <string.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/i2c.h>
#include <drivers/pwm.h>
#include <drivers/gpio.h>
#include <drivers/adc.h>
#include <drivers/counter.h>

#include <sys/printk.h>
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

/* FoC driver regulation thread */
static void foc_driver_regulation_loop(void *arg);
K_THREAD_DEFINE(reg_loop_tid, 4096,
		foc_driver_regulation_loop, NULL, NULL, NULL,
		0, 0, 0);

K_SEM_DEFINE(foc_trigger, 0, 1);

static void adc_work_handler(struct k_work *work);
K_WORK_DEFINE(adc_wq, adc_work_handler);

static enum adc_action foc_adc_measurement_handler(const struct device *dev,
						 const struct adc_sequence *sequence,
						 uint16_t sampling_index);

static float foc_adc_to_amperes_ratio = 1.0f;
static uint8_t sample_iabc_index = 0;
static int16_t adc_offsets[2] = {0,};
static int16_t adc_iabc_buffer[2];

static const struct adc_sequence_options iabc_options = {
	.interval_us = 0,
	.callback = foc_adc_measurement_handler,
	.extra_samplings = 0,
};

static const struct adc_sequence ia_sequence = {
	.options 	 = &iabc_options,
    .channels    = BIT(1),
    .buffer      = &adc_iabc_buffer[0],
    .buffer_size = sizeof(int16_t),
    .resolution  = 12,
};

static const struct adc_sequence ib_sequence = {
	.options 	 = &iabc_options,
    .channels    = BIT(7),
    .buffer      = &adc_iabc_buffer[1],
    .buffer_size = sizeof(int16_t),
    .resolution  = 12,
};

static const struct adc_channel_cfg ia_channel_cfg = {
	.gain             = ADC_GAIN_1,
	.reference        = ADC_REF_INTERNAL,
	.acquisition_time = ADC_ACQ_TIME_DEFAULT,
	.channel_id       = 1,
};

static const struct adc_channel_cfg ib_channel_cfg = {
	.gain             = ADC_GAIN_1,
	.reference        = ADC_REF_INTERNAL,
	.acquisition_time = ADC_ACQ_TIME_DEFAULT,
	.channel_id       = 7,
};

static const float encoder_to_degrees_ratio = (360.0f) / AS5600_PULSES_PER_REVOLUTION;
static const uint32_t pwm_period_half = (PWM_PERIOD_NSEC / 4);

static foc_measurement_callback_t user_callback;
static float pole_pairs;

static struct {
	uint16_t raw;
	uint16_t zero_offset;
} encoder_reading;

static const struct device *inverter_pwm = DEVICE_DT_GET(DT_NODELABEL(inverter_pwm));
static const struct device *encoder_i2c = DEVICE_DT_GET(DT_NODELABEL(i2c1));
static const struct device *current_sensors_adc = DEVICE_DT_GET(DT_NODELABEL(adc1));

static struct gpio_dt_spec enable_u = GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(inverter_enable),gpios,0);
static struct gpio_dt_spec enable_v = GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(inverter_enable),gpios,1);
static struct gpio_dt_spec enable_w = GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(inverter_enable),gpios,2);
static struct gpio_dt_spec sync_signal = GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(sync_signal),gpios,1);
static struct gpio_dt_spec instrumentation_signal = GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(sync_signal),gpios,0);

static struct gpio_callback pwm_sync_callback;
static struct step_measurement *bldc_measurement;

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

static void foc_driver_get_abc_currents(struct step_measurement * rotor_measurement)
{
	struct foc_controller_payload *p = rotor_measurement->payload;

	p->state.i_abc[0] = (float)(adc_offsets[0] - adc_iabc_buffer[0]) * foc_adc_to_amperes_ratio;
	p->state.i_abc[1] = (float)(adc_offsets[1] - adc_iabc_buffer[1]) * foc_adc_to_amperes_ratio; 

	/* reconstruct the third current */
	p->state.i_abc[2] = -(p->state.i_abc[0] + p->state.i_abc[1]);
}

static void foc_capture_isr(const struct device *port,
					struct gpio_callback *cb,
					gpio_port_pins_t pins)
{
	sample_iabc_index = 0;
	k_work_submit(&adc_wq);
}

static enum adc_action foc_adc_measurement_handler(const struct device *dev,
						 const struct adc_sequence *sequence,
						 uint16_t sampling_index)
{	
	sample_iabc_index++;
	k_work_submit(&adc_wq);
	return ADC_ACTION_FINISH;
}

static void adc_work_handler(struct k_work *work) 
{
	// if (!sample_iabc_index) {
	// 	adc_read_async(current_sensors_adc, &ia_sequence, NULL);
	// } else if (sample_iabc_index == 1) {
	// 	adc_read_async(current_sensors_adc, &ib_sequence, NULL);
	// } else {
		k_sem_give(&foc_trigger);
	// }
}

static void foc_driver_regulation_loop(void *arg)
{	
	int mcnt;
	bldc_measurement = step_sp_alloc(ROTOR_DRV_PAYLOAD_SZ);

	for(;;) {
		k_sem_take(&foc_trigger, K_FOREVER);

		gpio_pin_set_dt(&instrumentation_signal, 1);
		//foc_driver_get_rotor_position(bldc_measurement);
		foc_driver_get_abc_currents(bldc_measurement);

		if(user_callback) {
			user_callback(bldc_measurement);
		}

		/* fill measurements information before publishing: */
		bldc_measurement->header.filter_bits = rotor_sensor_header.filter_bits;
		bldc_measurement->header.unit_bits = rotor_sensor_header.unit_bits;
		bldc_measurement->header.srclen_bits = rotor_sensor_header.srclen_bits;

		/* publish sensor measurement to be processed by the waiters
		* in that case, FoC pipeline would be one of the waiters 
		*/
		step_sp_put(bldc_measurement);
		step_pm_poll(&mcnt, false);
		gpio_pin_set_dt(&instrumentation_signal, 0);
	}
}

int foc_driver_initialize(float adc_to_amperes_ratio, float motor_pole_pairs, foc_measurement_callback_t cb)
{
	int rc = 0;

	user_callback = cb;
	pole_pairs = motor_pole_pairs;
	foc_adc_to_amperes_ratio = adc_to_amperes_ratio;

	encoder_reading.raw = 0;
	encoder_reading.zero_offset = 0;

	i2c_configure(encoder_i2c, I2C_MODE_MASTER | I2C_SPEED_FAST);
	foc_driver_set_duty_cycle(0.0f, 0.0f, 0.0f);

	adc_channel_setup(current_sensors_adc, &ia_channel_cfg);
	adc_channel_setup(current_sensors_adc, &ib_channel_cfg);

	gpio_pin_configure_dt(&enable_u, GPIO_OUTPUT);
	gpio_pin_configure_dt(&enable_v, GPIO_OUTPUT);
	gpio_pin_configure_dt(&enable_w, GPIO_OUTPUT);
	gpio_pin_configure_dt(&instrumentation_signal, GPIO_OUTPUT);

	gpio_pin_set_dt(&enable_u, 1);
	gpio_pin_set_dt(&enable_v, 1);
	gpio_pin_set_dt(&enable_w, 1);
	gpio_pin_set_dt(&instrumentation_signal, 0);

	/* sets the adc offsets by reading them before enabling any motor phase */
	adc_read(current_sensors_adc, &ia_sequence);
	adc_read(current_sensors_adc, &ib_sequence);
	adc_offsets[0] = adc_iabc_buffer[0];
	adc_offsets[1] = adc_iabc_buffer[1];

	gpio_pin_configure_dt(&sync_signal, GPIO_INPUT);
	gpio_pin_interrupt_configure_dt(&sync_signal, GPIO_INT_EDGE_FALLING);
	gpio_init_callback(&pwm_sync_callback, foc_capture_isr, BIT(sync_signal.pin));
	gpio_add_callback(sync_signal.port, &pwm_sync_callback);
	pwm_set(inverter_pwm, 4 , PWM_PERIOD_NSEC, pwm_period_half, 0);

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
