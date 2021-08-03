/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include <step/step.h>
#include <step/filter.h>
#include <step/measurement/measurement.h>
#include <step/node.h>
#include "data.h"

/* Track callback entry statistics. */
struct step_test_data_procnode_cb_stats step_test_data_cb_stats = { 0 };

bool data_node_evaluate(struct step_measurement *mes, uint32_t handle,
			uint32_t inst)
{
	/* Overrides the filter engine when evaluating this node. */
	step_test_data_cb_stats.evaluate++;

	return true;
}

bool data_node_matched(struct step_measurement *mes, uint32_t handle,
		       uint32_t inst)
{
	/* Fires when the filter engine has indicated a match for this node. */
	step_test_data_cb_stats.matched++;

	return true;
}

int data_node_start(struct step_measurement *mes, uint32_t handle,
		    uint32_t inst)
{
	/* Fires before the node runs. */
	step_test_data_cb_stats.start++;

	return 0;
}

int data_node_exec(struct step_measurement *mes, uint32_t handle,
		   uint32_t inst)
{
	/* Node logic implementation. */
	step_test_data_cb_stats.run++;

	return 0;
}

int data_node_stop(struct step_measurement *mes, uint32_t handle,
		   uint32_t inst)
{
	/* Fires when the node has been successfully run. */
	step_test_data_cb_stats.stop++;

	return 0;
}

void data_node_error(struct step_measurement *mes, uint32_t handle,
		     uint32_t inst, int error)
{
	/* Fires when an error occurs running this node. */
	step_test_data_cb_stats.error++;
}

/* Processor node chain. */
static struct step_node step_test_data_procnode_chain_data[] = {
	/* Processor node 0. */
	{
		.name = "Temperature chain",

		/* Temperature filter. */
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
					/* Make sure timestamp (bits 26-28) = EPOCH32 */
					.op = STEP_FILTER_OP_AND,
					.match = (STEP_MES_TIMESTAMP_EPOCH_32 <<
						  STEP_MES_MASK_TIMESTAMP_POS),
					.ignore_mask = ~STEP_MES_MASK_TIMESTAMP,
				},
			},
		},

		/* Callbacks */
		.callbacks = {
			.evaluate_handler = NULL,
			.matched_handler = data_node_matched,
			.start_handler = data_node_start,
			.stop_handler = data_node_stop,
			.exec_handler = data_node_exec,
			.error_handler = data_node_error,
		},

		/* Config settings */
		.config = NULL,

		/* Point to next processor node in chain. */
		.next = &step_test_data_procnode_chain_data[1]
	},
	/* Processor node 1. */
	{
		.name = "Secondary node",

		/* Callbacks */
		.callbacks = {
			.evaluate_handler = data_node_evaluate,
			.matched_handler = data_node_matched,
			.start_handler = data_node_start,
			.stop_handler = data_node_stop,
			.exec_handler = data_node_exec,
			.error_handler = data_node_error,
		},

		/* Config settings */
		.config = NULL,

		/* End of the chain. */
		.next = NULL
	}
};

/* Pointer to node chain. */
struct step_node *step_test_data_procnode_chain =
	step_test_data_procnode_chain_data;

/* Single processor node. */
struct step_node step_test_data_procnode = {
	.name = "Temperature",
	/* Matches if DS filter field = 0x26 OR 0x226 AND bit 26 is set. */
	.filters = {
		.count = 3,
		.chain = (struct step_filter[]){
			{
				/* Temperature (base type). */
				.match = STEP_MES_TYPE_TEMPERATURE,
				.ignore_mask = ~STEP_MES_MASK_FULL_TYPE,         /* 0xFFFF0000 */
			},
			{
				/* Die temperature. */
				.op = STEP_FILTER_OP_OR,
				.match = STEP_MES_TYPE_TEMPERATURE +
					 (STEP_MES_EXT_TYPE_TEMP_DIE <<
					  STEP_MES_MASK_EXT_TYPE_POS),
				.ignore_mask = ~STEP_MES_MASK_FULL_TYPE,         /* 0xFFFF0000 */
			},
			{
				/* Make sure timestamp (bits 26-28) = EPOCH32. */
				.op = STEP_FILTER_OP_AND,
				.match = (STEP_MES_TIMESTAMP_EPOCH_32 <<
					  STEP_MES_MASK_TIMESTAMP_POS),
				.ignore_mask = ~STEP_MES_MASK_TIMESTAMP,         /* 0xE3FFFFFF */
			},
		},
	},

	/* Callbacks */
	.callbacks = {
		.evaluate_handler = data_node_evaluate,
		.matched_handler = data_node_matched,
		.start_handler = data_node_start,
		.stop_handler = data_node_stop,
		.exec_handler = data_node_exec,
		.error_handler = data_node_error,
	},

	/* Config settings */
	.config = (void *)0x12345678,

	/* Not a chain. */
	.next = NULL
};

/* Die temperature with 32-bit timestamp payload. */
struct {
	uint32_t timestamp;
	float temp_c;
} step_test_data_dietemp_payload = {
	.timestamp = 1624305803,         /* Monday, June 21, 2021 8:03:23 PM */
	.temp_c = 32.0F,
};

/* Test die temp measurement, with timestamp. */
struct step_measurement step_test_mes_dietemp = {
	/* Measurement metadata. */
	.header = {
		/* Filter word. */
		.filter = {
			.base_type = STEP_MES_TYPE_TEMPERATURE,
			.ext_type = STEP_MES_EXT_TYPE_TEMP_DIE,
			.flags = {
				.timestamp = STEP_MES_TIMESTAMP_EPOCH_32,
			},
		},
		/* SI Unit word. */
		.unit = {
			.si_unit = STEP_MES_UNIT_SI_DEGREE_CELSIUS,
			.ctype = STEP_MES_UNIT_CTYPE_IEEE754_FLOAT32,
		},
		/* Source/Len word. */
		.srclen = {
			.len = sizeof(step_test_data_dietemp_payload),
			.sourceid = 10,
		},
	},

	/* Die temperature in C plus 32-bit epoch timestamp. */
	.payload = &step_test_data_dietemp_payload,
};
