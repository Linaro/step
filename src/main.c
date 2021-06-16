/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include "sample_pool.h"
#include "proc_mgr.h"

bool node_evaluate(struct sdp_datasample *sample, void *cfg)
{
	/* Overrides the filter engine when evaluating this node. */
	return true;
}

bool node_matched(struct sdp_datasample *sample, void *cfg)
{
	/* Fires when the filter engine has indicated a match for this node. */
	return true;
}

int node_start(struct sdp_datasample *sample, void *cfg)
{
	/* Fires before the node runs. */
	return 0;
}

int node_run(struct sdp_datasample *sample, void *cfg)
{
	/* Node logic implementation. */
	return 0;
}

int node_stop(struct sdp_datasample *sample, void *cfg)
{
	/* Fires when the node has been successfully run. */
	return 0;
}

void node_error(struct sdp_datasample *sample, void *cfg, int error)
{
	/* Fires when an error occurs running this node. */
}

static struct sdp_node g_test_proc_node = {
	/* Matches if DS filter field = (0x26 OR 0x226) AND bit 17 is set. */
	.filter_count = 3,
	.filters = (struct sdp_filter[]){
		{
			.exact_match = SDP_DS_TYPE_TEMPERATURE,
		},
		{
			.comb_op = SDP_FILTER_COMB_OP_OR,
			.exact_match = SDP_DS_TYPE_TEMPERATURE + (SDP_DS_EXT_TYPE_TEMP_DIE << 8),
		},
		{
			.comb_op = SDP_FILTER_COMB_OP_AND,
			.bits = {
				.mask = 0xFFFDFFFF,
				.set = (1 << 17)
			}
		}
	},

	/* Config settings */
	.config = (void *)0x12345678,

	/* Callbacks */
	.evaluate_handler = node_evaluate,
	.matched_handler = node_matched,
	.start_handler = node_start,
	.run_handler = node_run,
	.stop_handler = node_stop,
	.error_handler = node_error
};

static struct sdp_datasample g_test_datasample = {
	.header = {
		/* Filter word. */
		.filter.data_type = SDP_DS_TYPE_TEMPERATURE,
		.filter.ext_type = SDP_DS_EXT_TYPE_TEMP_DIE,
		.filter.flags = {
			.data_format = SDP_DS_FORMAT_NONE,
			.encoding = SDP_DS_ENCODING_NONE,
			.encrypt_alg = SDP_DS_ENCRYPT_ALG_NONE,
		},
		/* Source/Len word. */
		.srclen.len = 4,
		.srclen.is_partial = 1,
		.srclen.timestamp = 1,
		.srclen.sourceid = 10
	},
	.payload = NULL
};

int main()
{
	uint32_t rc = 0;
	uint8_t pm_handle = 0;

	/* Register the processor node. */
	rc = sdp_pm_register(&g_test_proc_node, &pm_handle);

	/* Add a sample to the sample pool. */
	rc = sdp_sp_add(&g_test_datasample);

	return 0;
}
