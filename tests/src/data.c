/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include <sdp/sdp.h>
#include <sdp/filter.h>
#include <sdp/measurement/measurement.h>
#include <sdp/node.h>
#include "data.h"

/* Track callback entry statistics. */
struct sdp_test_data_procnode_cb_stats sdp_test_data_cb_stats = { 0 };

bool data_node_evaluate(struct sdp_measurement *mes, void *cfg)
{
	/* Overrides the filter engine when evaluating this node. */
	sdp_test_data_cb_stats.evaluate++;

	return true;
}

bool data_node_matched(struct sdp_measurement *mes, void *cfg)
{
	/* Fires when the filter engine has indicated a match for this node. */
	sdp_test_data_cb_stats.matched++;

	return true;
}

int data_node_start(struct sdp_measurement *mes, void *cfg)
{
	/* Fires before the node runs. */
	sdp_test_data_cb_stats.start++;

	return 0;
}

int data_node_run(struct sdp_measurement *mes, void *cfg)
{
	/* Node logic implementation. */
	sdp_test_data_cb_stats.run++;

	return 0;
}

int data_node_stop(struct sdp_measurement *mes, void *cfg)
{
	/* Fires when the node has been successfully run. */
	sdp_test_data_cb_stats.stop++;

	return 0;
}

void data_node_error(struct sdp_measurement *mes, void *cfg, int error)
{
	/* Fires when an error occurs running this node. */
	sdp_test_data_cb_stats.error++;
}

/* Processor node chain. */
static struct sdp_node sdp_test_data_procnode_chain_data[] = {
	/* Processor node 0. */
	{
		.name = "Temperature chain",

		/* Temperature filter. */
		.filters = {
			.count = 3,
			.chain = (struct sdp_filter[]){
				{
					/* Temperature (base type). */
					.match = SDP_MES_TYPE_TEMPERATURE,
					.ignore_mask = ~SDP_MES_MASK_FULL_TYPE,
				},
				{
					/* Die temperature. */
					.op = SDP_FILTER_OP_OR,
					.match = SDP_MES_TYPE_TEMPERATURE +
						 (SDP_MES_EXT_TYPE_TEMP_DIE <<
						  SDP_MES_MASK_EXT_TYPE_POS),
					.ignore_mask = ~SDP_MES_MASK_FULL_TYPE,
				},
				{
					/* Make sure timestamp (bits 26-28) = EPOCH32 */
					.op = SDP_FILTER_OP_AND,
					.match = (SDP_MES_TIMESTAMP_EPOCH_32 <<
						  SDP_MES_MASK_TIMESTAMP_POS),
					.ignore_mask = ~SDP_MES_MASK_TIMESTAMP,
				},
			},
		},

		/* Callbacks */
		.callbacks = {
			.evaluate_handler = NULL,
			.matched_handler = data_node_matched,
			.start_handler = data_node_start,
			.stop_handler = data_node_stop,
			.run_handler = data_node_run,
			.error_handler = data_node_error,
		},

		/* Config settings */
		.config = NULL,

		/* Point to next processor node in chain. */
		.next = &sdp_test_data_procnode_chain_data[1]
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
			.run_handler = data_node_run,
			.error_handler = data_node_error,
		},

		/* Config settings */
		.config = NULL,

		/* End of the chain. */
		.next = NULL
	}
};

/* Pointer to node chain. */
struct sdp_node *sdp_test_data_procnode_chain =
	sdp_test_data_procnode_chain_data;

/* Single processor node. */
struct sdp_node sdp_test_data_procnode = {
	.name = "Temperature",
	/* Matches if DS filter field = 0x26 OR 0x226 AND bit 26 is set. */
	.filters = {
		.count = 3,
		.chain = (struct sdp_filter[]){
			{
				/* Temperature (base type). */
				.match = SDP_MES_TYPE_TEMPERATURE,
				.ignore_mask = ~SDP_MES_MASK_FULL_TYPE,         /* 0xFFFF0000 */
			},
			{
				/* Die temperature. */
				.op = SDP_FILTER_OP_OR,
				.match = SDP_MES_TYPE_TEMPERATURE +
					 (SDP_MES_EXT_TYPE_TEMP_DIE <<
					  SDP_MES_MASK_EXT_TYPE_POS),
				.ignore_mask = ~SDP_MES_MASK_FULL_TYPE,         /* 0xFFFF0000 */
			},
			{
				/* Make sure timestamp (bits 26-28) = EPOCH32. */
				.op = SDP_FILTER_OP_AND,
				.match = (SDP_MES_TIMESTAMP_EPOCH_32 <<
					  SDP_MES_MASK_TIMESTAMP_POS),
				.ignore_mask = ~SDP_MES_MASK_TIMESTAMP,         /* 0xE3FFFFFF */
			},
		},
	},

	/* Callbacks */
	.callbacks = {
		.evaluate_handler = data_node_evaluate,
		.matched_handler = data_node_matched,
		.start_handler = data_node_start,
		.stop_handler = data_node_stop,
		.run_handler = data_node_run,
		.error_handler = data_node_error,
	},

	/* Config settings */
	.config = (void *)0x12345678,

	/* Not a chain. */
	.next = NULL
};

/* Die temperature with 32-bit timestamp payload. */
struct {
	float temp_c;
	uint32_t timestamp;
} sdp_test_data_dietemp_payload = {
	.temp_c = 32.0F,
	.timestamp = 1624305803,         /* Monday, June 21, 2021 8:03:23 PM */
};

/* Test die temp measurement, with timestamp. */
struct sdp_measurement sdp_test_mes_dietemp = {
	/* Measurement metadata. */
	.header = {
		/* Filter word. */
		.filter = {
			.base_type = SDP_MES_TYPE_TEMPERATURE,
			.ext_type = SDP_MES_EXT_TYPE_TEMP_DIE,
			.flags = {
				.timestamp = SDP_MES_TIMESTAMP_EPOCH_32,
			},
		},
		/* SI Unit word. */
		.unit = {
			.si_unit = SDP_MES_UNIT_SI_DEGREE_CELSIUS,
			.ctype = SDP_MES_UNIT_CTYPE_IEEE754_FLOAT32,
		},
		/* Source/Len word. */
		.srclen = {
			.len = sizeof(sdp_test_data_dietemp_payload),
			.sourceid = 10,
		},
	},

	/* Die temperature in C plus 32-bit epoch timestamp. */
	.payload = &sdp_test_data_dietemp_payload,
};
