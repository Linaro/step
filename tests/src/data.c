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

bool node_evaluate(struct sdp_measurement *mes, void *cfg)
{
	/* Overrides the filter engine when evaluating this node. */
	sdp_test_data_cb_stats.evaluate++;

	return true;
}

bool node_matched(struct sdp_measurement *mes, void *cfg)
{
	/* Fires when the filter engine has indicated a match for this node. */
	sdp_test_data_cb_stats.matched++;

	return true;
}

int node_start(struct sdp_measurement *mes, void *cfg)
{
	/* Fires before the node runs. */
	sdp_test_data_cb_stats.start++;

	return 0;
}

int node_run(struct sdp_measurement *mes, void *cfg)
{
	/* Node logic implementation. */
	sdp_test_data_cb_stats.run++;

	return 0;
}

int node_stop(struct sdp_measurement *mes, void *cfg)
{
	/* Fires when the node has been successfully run. */
	sdp_test_data_cb_stats.stop++;

	return 0;
}

void node_error(struct sdp_measurement *mes, void *cfg, int error)
{
	/* Fires when an error occurs running this node. */
	sdp_test_data_cb_stats.error++;
}

/* Test processor node. */
struct sdp_node sdp_test_data_procnode = {
	/* Matches if DS filter field = 0x26 OR 0x226 AND bit 26 is set. */
	.filters = {
		.count = 3,
		.chain = (struct sdp_filter[]){
			{
				.match = SDP_MES_TYPE_TEMPERATURE,
				.ignore_mask = ~SDP_MES_MASK_FULL_TYPE, /* 0xFFFF0000 */
			},
			{
				.op = SDP_FILTER_OP_OR,
				.match = SDP_MES_TYPE_TEMPERATURE +
					 (SDP_MES_EXT_TYPE_TEMP_DIE <<
					  SDP_MES_MASK_EXT_TYPE_POS),
				.ignore_mask = ~SDP_MES_MASK_FULL_TYPE, /* 0xFFFF0000 */
			},
			{
				/* Make sure timestamp (bits 26-28) = EPOCH32 */
				.op = SDP_FILTER_OP_AND,
				.match = (SDP_MES_TIMESTAMP_EPOCH_32 <<
					  SDP_MES_MASK_TIMESTAMP_POS),
				.ignore_mask = ~SDP_MES_MASK_TIMESTAMP, /* 0xE3FFFFFF */
			},
		},
	},

	/* Callbacks */
	.callbacks = {
		.evaluate_handler = node_evaluate,
		.matched_handler = node_matched,
		.start_handler = node_start,
		.stop_handler = node_stop,
		.run_handler = node_run,
		.error_handler = node_error,
	},

	/* Config settings */
	.config = (void *)0x12345678,
};

/* Die temperature with 32-bit timestamp payload. */
struct {
	float temp_c;
	uint32_t timestamp;
} sdp_test_data_dietemp_payload = {
	.temp_c = 32.0F,
	.timestamp = 1624305803,        /* Monday, June 21, 2021 8:03:23 PM */
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
