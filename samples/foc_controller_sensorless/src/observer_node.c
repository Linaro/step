/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <stdio.h>
#include <logging/log.h>
#include <step/proc_mgr.h>
#include <arm_math.h>
#include <math.h>
#include "observer_node.h"
#include "foc_driver.h"

/* Squared */
#define SQUARE(x)   ((x) * (x))

/* observer may enter in NAN values */
#define IS_NAN(x)   ((x) != (x))
#define NAN_ZERO(x) (x = IS_NAN(x) ? 0.0 : x)

static const float cycle_to_sec_ratio = 1.0f / (float)CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC;
static const float rad_to_degrees_ratio = (180.0f) / PI;

static struct {
	float xt_1;
	float xt_2;
	float omega_hat;
	float last_dt;		
} bldc_observer;

int observer_initialize(void *cfg, uint32_t handle, uint32_t inst)
{
	bldc_observer.xt_1 = 0.0f;
	bldc_observer.xt_2 = 0.0f;
	bldc_observer.omega_hat = 0.0f;
	bldc_observer.last_dt = cycle_to_sec_ratio * (float)k_cycle_get_32();
    return 0;
}

int observer_estimate(struct step_measurement *mes, uint32_t handle, uint32_t inst)
{
	struct foc_controller_payload *p = mes->payload;
	struct foc_feeback_sensor_data *s = &p->state;
	struct foc_command_data *cmd = &p->cmd;

	float xt_dot_1;
	float xt_dot_2;
	float error;
	float l_stator[2];
	float r_stator[2];
	float xt_norm = 0.0f;
	float now_secs = cycle_to_sec_ratio * (float)k_cycle_get_32();
	float dt = now_secs - bldc_observer.last_dt;
	bldc_observer.last_dt = now_secs;

	/* gets the i_alpha_beta currents by doing clarke transform */
	arm_clarke_f32(s->i_abc[0], s->i_abc[1], &s->i_alpha_beta[0], &s->i_alpha_beta[1]);

	/* first, compute the stator inductance plus resistance */
	l_stator[0] = 1.5f * motor_observer_node_conf.motor_phase_inductance * s->i_alpha_beta[0];
	l_stator[1] = 1.5f * motor_observer_node_conf.motor_phase_inductance * s->i_alpha_beta[1];
	r_stator[0] = 1.5f * motor_observer_node_conf.motor_phase_resistance * s->i_alpha_beta[0];
	r_stator[1] = 1.5f * motor_observer_node_conf.motor_phase_resistance * s->i_alpha_beta[1];

	/* compute estimation error */
	error = SQUARE(motor_observer_node_conf.motor_flux_linkage) - 
		(SQUARE(bldc_observer.xt_1 - l_stator[0]) - SQUARE(bldc_observer.xt_2 - l_stator[1]));

	/* force error to be positive to speed-up the convergence */
	if(error < 0.0f) {
		error = 0.0f;
	}

	/* update the observer states */
	xt_dot_1 = -r_stator[0] + cmd->voltage_alpha + 
		motor_observer_node_conf.observer_gain * 
		(bldc_observer.xt_1 - l_stator[0]) * error;

	xt_dot_2 = -r_stator[1] + cmd->voltage_beta + 
		motor_observer_node_conf.observer_gain * 
		(bldc_observer.xt_2 - l_stator[1]) * error;

	bldc_observer.xt_1 += xt_dot_1 * dt;
	bldc_observer.xt_2 += xt_dot_2 * dt;

	NAN_ZERO(bldc_observer.xt_1);
	NAN_ZERO(bldc_observer.xt_2);

	/* prevent loss of stabillity when states gets too low in magnitude */
	arm_sqrt_f32(SQUARE(bldc_observer.xt_1) + SQUARE(bldc_observer.xt_2), &xt_norm);
	if (xt_norm < (motor_observer_node_conf.motor_flux_linkage * 0.5)) {
		bldc_observer.xt_1 *= 1.1;
		bldc_observer.xt_2 *= 1.1;
	}

	/* estimate angle using the output states from observer */
	bldc_observer.omega_hat = atan2f(bldc_observer.xt_2 - l_stator[1], 
								bldc_observer.xt_1 - l_stator[0]) *
								rad_to_degrees_ratio;
	
	s->estimated_rotor_position = bldc_observer.omega_hat;
	s->dt = dt;

	return 0;
}

/* Processor node chain. */
struct step_node observer_node_chain_data[] = {
	/* Root processor node. */
	{
		.name = "observer root node",
		.filters = {
			.count = 1,
			.chain = (struct step_filter[]){
				{
					/* Phase (base type). */
					.match = STEP_MES_TYPE_PHASE,
					.ignore_mask = ~STEP_MES_MASK_FULL_TYPE,
				}
			},
		},

		/* Callbacks */
		.callbacks = {
			.init_handler = observer_initialize,
		},

		/* Config settings */
		.config = NULL,

		/* Point to next processor node in chain. */
		.next = &observer_node_chain_data[1]
	},

	/* Processor node 1. */
	{
		.name = "observer processing node",
		/* Callbacks */
		.callbacks = {
			.exec_handler = observer_estimate,
		},

		/* Config settings */
		.config = NULL,

		/* Point to next processor node in chain. */
		.next = NULL,
	},
};

/* Pointer to node chain. */
struct step_node *observer_node_chain = observer_node_chain_data;
struct motor_observer_node_config motor_observer_node_conf = {
	.observer_gain = 0.5f,
};
