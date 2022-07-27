/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <stdio.h>
#include <logging/log.h>
#include <step/proc_mgr.h>


int sensor_do_process(struct step_measurement *mes, uint32_t handle, uint32_t inst)
{
	return 0;
}

/* Processor node chain. */
struct step_node sensor_node_chain_data[] = {
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
			.exec_handler = sensor_do_process,
		},

		/* Config settings */
		.config = NULL,

		/* Point to next processor node in chain. */
		.next = NULL,
	},
};

/* Pointer to node chain. */
struct step_node *sensor_node_chain = sensor_node_chain_data;
