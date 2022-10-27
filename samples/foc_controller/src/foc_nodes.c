/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <stdio.h>
#include <zephyr/logging/log.h>
#include <step/proc_mgr.h>
#include "foc_nodes.h"
#include "foc_driver.h"
#include "foc_svm.h"
#include <arm_math.h>

/**
 *  ┌───────────────┐
 *  │               │
 *  │ Rotor Sensor  │          This is the FoC pipeline created using STeP:
 *  │ (I2C)         │       1. Collect node which is trigggered by rotor position sensor driver
 *  │               │       2. Inverse Park transfor to map VQD to rotating VaplhaBeta
 *  │               │       3. Inverse Clarke transform to project ValphaBeta into 3-phase vector
 *  └──────┬────────┘       4. Inverter update node that sends 3-phase vector to the PWM driver
 *         │
 *         │                The STeP node chain is created to represent the block diagram
 *         │                below.
 *         │
 *         │
 *         │           ---------------------------------------------------
 *         │		   | FoC algorithm Node								 |						 	
 * ┌───────▼────────┐  | ┌────────────────┐            ┌───────────────┐ |       ┌────────────────┐
 * │                │  | │                │            │               │ |       │                │
 * │  Collect       │  | │ Inverse        │            │ Inverse       │ |       │ Inverter       │
 * │  Node          │  | │ Park Transform │            │ Clarke Trans. │ |       │ Update         │
 * │                ├───►│                ├────────────►               ├────────►│ Node           │
 * │  Capture       │  | │ VQD -> Vab     │            │ Vab->Vuvw     │ |       │ Vuvw-> SVPWM   │
 * │  the samples   │  | │                │            │               │ |       │                │
 * │                │  | │                │            │               │ |       │                │
 * └───────▲────────┘  | └────────────────┘            └───────────────┘ |       └────────┬───────┘
 *         │           |                                                 |                │
 *         │           |-------------------------------------------------|                │
 *         │                                                                              ▼
 *         │                                                                     ┌────────────────┐
 *  ┌──────┴────────┐                                                            │                │
 *  │               │                                                            │ Inverter       │
 *  │ Desired       │                                                            │ Driver         │
 *  │ Voltage       │                                                            │                │
 *  │ (Shell)       │                                                            │ (PWM)          │
 *  │               │                                                            │                │
 *  │               │                                                            └────────────────┘
 *  └───────────────┘
 */

/* rotor sensor align step */
int foc_align_rotor(void *cfg, uint32_t handle, uint32_t inst)
{
	/* for rotor alignment the idea is to provide the initial voltage
	 * vector with a phase of 90 degree and apply to the motor, and wait
	 * the rotor to align mechanically to that voltage applied, this will
	 * translate the stator and rotor magnetic fields are properly 90 degree
	 * aligned
	 */
	float sine;
	float cosine;
	float alpha;
	float beta;
	float inverter_duties[3];

	/* we want a voltage vector with 90 degree of phase */
	arm_sin_cos_f32(90, &sine, &cosine);

	/* use only 20% of the current capacity to avoid heating*/
	arm_inv_park_f32(0.0f,
					0.1f,
					&alpha,
					&beta,
					sine,
					cosine);

	/* convert the alpha-beta frame into SVPWM signals */
	foc_svm_set(alpha, 
			beta,
			&inverter_duties[0],
			&inverter_duties[1],
			&inverter_duties[2]);

	/* set the voltage vector to the inverter and... */
	foc_driver_set_duty_cycle(inverter_duties[0], inverter_duties[1], inverter_duties[2]);
	
	/* give a delay to the rotor align mechanically */
	k_sleep(K_MSEC(500));

	/* before releasing the voltage, sets the zero angle offset in the encoder*/
	return foc_driver_set_encoder_offset();
}

/* foc algorithm process step */
int foc_do_process(struct step_measurement *mes, uint32_t handle, uint32_t inst)
{
	struct foc_controller_payload *p = mes->payload;
	struct foc_command_data *command = &p->cmd;
	struct foc_feeback_sensor_data *feed = &p->state;
	float sine;
	float cosine;

	arm_sin_cos_f32(feed->e_rotor_position, &sine, &cosine);

	arm_inv_park_f32(command->voltage_d,
					command->voltage_q,
					&command->voltage_alpha,
					&command->voltage_beta,
					sine,
					cosine);
	
	return 0;
}

/* do SVPWM update the inverter step, the last block on FoC chain */
int foc_do_space_vector_modulation(struct step_measurement *mes, uint32_t handle, uint32_t inst)
{
	struct foc_controller_payload *p = mes->payload;
	float inverter_duties[3];

	/* convert the alpha-beta frame into SVPWM signals */
	foc_svm_set(p->cmd.voltage_alpha, 
			p->cmd.voltage_beta,
			&inverter_duties[0],
			&inverter_duties[1],
			&inverter_duties[2]);

	/* and transfer them directly to the inverter */
	foc_driver_set_duty_cycle(inverter_duties[0], inverter_duties[1], inverter_duties[2]);
	
	return 0;
}

/* Processor node chain. */
struct step_node foc_node_chain_data[] = {
	/* Root processor node. */
	{
		.name = "Root sensor sample processor node",
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
			.init_handler = foc_align_rotor,
		},

		/* Config settings */
		.config = NULL,

		/* Point to next processor node in chain. */
		.next = &foc_node_chain_data[1]
	},

	/* Processor node 1. */
	{
		.name = "foc processing node",
		/* Callbacks */
		.callbacks = {
			.exec_handler = foc_do_process,
		},

		/* Config settings */
		.config = NULL,

		/* Point to next processor node in chain. */
		.next = &foc_node_chain_data[2]
	},

	/* Processor node 2. */
	{
		.name = "inverter update",
		/* Callbacks */
		.callbacks = {
			.exec_handler = foc_do_space_vector_modulation,
		},

		/* Config settings */
		.config = NULL,

		/* End of the chain. */
		.next = NULL
	},
};

/* Pointer to node chain. */
struct step_node *foc_node_chain = foc_node_chain_data;
