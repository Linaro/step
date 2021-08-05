/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <logging/log.h>
#include <step/proc_mgr.h>
#include "nodes.h"
#include "driver.h"

LOG_MODULE_DECLARE(step_shell);

struct step_node_cb_stats cb_stats = { 0 };

/* Demonstrate how a custom config struct can be used with node callbacks. */
struct node_cfg {
	float mult;
} node_cfg_inst = {
	.mult = 10.0F,
};

/* Overrides the filter engine when evaluating this node. */
int node_init(void *cfg, uint32_t handle, uint32_t inst)
{
	cb_stats.init++;

	return 0;
}

/* Overrides the filter engine when evaluating this node. */
bool node_evaluate(struct step_measurement *mes, uint32_t handle, uint32_t inst)
{
	cb_stats.evaluate++;

	return true;
}

/* Fires when the filter engine has indicated a match for this node. */
bool node_matched(struct step_measurement *mes, uint32_t handle, uint32_t inst)
{
	cb_stats.matched++;

	return true;
}

/* Fires before the node runs. */
int node_start(struct step_measurement *mes, uint32_t handle, uint32_t inst)
{
	cb_stats.start++;

	return 0;
}

/* Main node processing function. */
int node_exec(struct step_measurement *mes, uint32_t handle, uint32_t inst)
{
	if (mes->header.filter.ext_type == STEP_MES_EXT_TYPE_TEMP_DIE) {
		struct step_node *node;
		float mult = 1.0F;

		/* Get a reference to the source node from the proc. node registry. */
		node = step_pm_node_get(handle, inst);
		if (node == NULL) {
			return -EINVAL;
		}

		/* Parse the config struct if present. */
		if (node->config != NULL) {
			struct node_cfg *c = node->config;
			mult = c->mult;
			LOG_INF("cfg: mult by %.2f (handle %d:%d)", c->mult, handle, inst);
		}

		/* Display the timestamp and die temp value. */
		struct drv_payload *tp = mes->payload;
		tp->temp_c *= mult;
		LOG_INF("[%d] Received die temp: %0.2f C (handle %d:%d)",
			tp->timestamp, tp->temp_c, handle, inst);
	} else {
		LOG_WRN("Unexpected message type: base=%02X ext=%02X, handle %d:%d",
			mes->header.filter.base_type, mes->header.filter.ext_type,
			handle, inst);
		return -EINVAL;
	}

	cb_stats.run++;
	return 0;
}

/* Fires when the node has been successfully run. */
int node_stop(struct step_measurement *mes, uint32_t handle, uint32_t inst)
{
	cb_stats.stop++;

	return 0;
}

/* Fires when an error occurs running this node. */
void node_error(struct step_measurement *mes, uint32_t handle,
		uint32_t inst, int error)
{
	cb_stats.error++;
}

/* Processor node chain. */
struct step_node test_node_chain_data[] = {
	/* Root processor node. */
	{
		.name = "Root processor node",
		.filters = {
			.count = 3,
			.chain = (struct step_filter[]){
				{
					/* Temperature (base type). */
					.match = STEP_MES_TYPE_TEMPERATURE,
					.ignore_mask = ~STEP_MES_MASK_FULL_TYPE,
				},
				{
					/* Die temperature. */
					.op = STEP_FILTER_OP_OR,
					.match = STEP_MES_TYPE_TEMPERATURE +
						 (STEP_MES_EXT_TYPE_TEMP_DIE <<
						  STEP_MES_MASK_EXT_TYPE_POS),
					.ignore_mask = ~STEP_MES_MASK_FULL_TYPE,
				},
				{
					/* Make sure timestamp (bits 26-28) = UPTIME_MS_32 */
					.op = STEP_FILTER_OP_AND,
					.match = (STEP_MES_TIMESTAMP_UPTIME_MS_32 <<
						  STEP_MES_MASK_TIMESTAMP_POS),
					.ignore_mask = ~STEP_MES_MASK_TIMESTAMP,
				},
			},
		},

		/* Callbacks */
		.callbacks = {
			.init_handler = node_init,
			.evaluate_handler = NULL,
			.matched_handler = node_matched,
			.start_handler = node_start,
			.stop_handler = node_stop,
			.exec_handler = node_exec,
			.error_handler = node_error,
		},

		/* Config settings */
		.config = NULL,

		/* Point to next processor node in chain. */
		.next = &test_node_chain_data[1]
	},

	/* Processor node 1. */
	{
		.name = "2nd processor node",
		/* Callbacks */
		.callbacks = {
			.init_handler = node_init,
			.exec_handler = node_exec,
			.error_handler = node_error,
		},

		/* Config settings (multiplication factor) */
		.config = &node_cfg_inst,

		/* Point to next processor node in chain. */
		.next = &test_node_chain_data[2]
	},

	/* Processor node 2. */
	{
		.name = "3rd processor node",
		/* Callbacks */
		.callbacks = {
			.init_handler = node_init,
			.exec_handler = node_exec,
			.error_handler = node_error,
		},

		/* Config settings */
		.config = NULL,

		/* End of the chain. */
		.next = NULL
	}
};

/* Pointer to node chain. */
struct step_node *test_node_chain = test_node_chain_data;
