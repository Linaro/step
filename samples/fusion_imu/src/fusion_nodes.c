/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <stdio.h>
#include <logging/log.h>
#include <step/proc_mgr.h>
#include <zsl/zsl.h>
#include <zsl/orientation/fusion/fusion.h>
#include <zsl/orientation/orientation.h>


#include "imu_data_producer.h"


int fusion_initialize_states(void *cfg, uint32_t handle, uint32_t inst)
{
	return 0;
}

int fusion_do_process(struct step_measurement *mes, uint32_t handle, uint32_t inst)
{
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
					.match = STEP_MES_TYPE_ORIENTATION,
					.ignore_mask = ~STEP_MES_MASK_FULL_TYPE,
				}
			},
		},

		/* Callbacks */
		.callbacks = {
			.init_handler = fusion_initialize_states,
		},

		/* Config settings */
		.config = NULL,

		/* Point to next processor node in chain. */
		.next = &foc_node_chain_data[1]
	},

	/* Processor node 1. */
	{
		.name = "fusion processing node",
		/* Callbacks */
		.callbacks = {
			.exec_handler = fusion_do_process,
		},

		/* Config settings */
		.config = NULL,

		/* Point to next processor node in chain. */
		.next = NULL,
	},
};

/* Pointer to node chain. */
struct step_node *fusion_node_chain = fusion_node_chain_data;
