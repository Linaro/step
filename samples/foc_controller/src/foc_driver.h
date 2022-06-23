/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef FOC_DRIVER_H__
#define FOC_DRIVER_H__

#include <step/step.h>
#include <step/measurement/measurement.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief FoC measurement reception callback:
 */
typedef void (*foc_measurement_callback_t) (struct step_measurement *mes);

/**
 * @brief FoC rotor position sensor data:
 */
struct foc_feeback_sensor_data {
	float motor_pole_pairs;
	float e_rotor_position;
	float rotor_position;
};

/**
 * @brief FoC voltages flow data:
 */
struct foc_command_data {
	float voltage_q;
	float voltage_d;
	float voltage_alpha;
	float voltage_beta;
	float voltage_u;
	float voltage_v;
};

/**
 * @brief FoC data payload descriptor:
 */
struct foc_controller_payload {
	uint32_t timestamp;
	struct foc_feeback_sensor_data state;
	struct foc_command_data cmd;
};

/**
 * @brief Initializes the foc driver.
 * 
 * @param cb user callback to be executed each measurement from motor rotor position.
 * 
 * @param motor_pole_pairs number of motor pole pairs.
 * 
 * @return int  0 on success, otherwise a negative error code.
 * 
 */
int foc_driver_initialize(float motor_pole_pairs, foc_measurement_callback_t cb);


/**
 * @brief Sets the 0-degree offset for the encoder
 * 
 * @return int  0 on success, otherwise a negative error code.
 * 
 */
int foc_driver_set_encoder_offset(void);

/**
 * @brief set inverter voltages based on duty cycle.
 * 
 * @param dc_u voltage to be applied to the in phase u of motor
 *
 * @param dc_v voltage to be applied to the in phase v of motor
 *
 * @param dc_v voltage to be applied to the in phase v of motor
 * 
 * @param values should be normalized between 0 and 1 
 */
void foc_driver_set_duty_cycle(float dc_u, float dc_v, float dc_w);

#ifdef __cplusplus
}
#endif

#endif /* STEP_SHELL_DRIVER_H_ */
