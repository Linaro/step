/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include "datasample.h"
#include "procmgr.h"

void print_header(struct sdp_ds_header *header)
{
	printk("Filter:           0x%08X\n", header->filter_bits);
	printk("  data_type:      0x%02X (%u)\n", header->filter.data_type, header->filter.data_type);
	printk("  ext_type:       0x%02X (%u)\n", header->filter.ext_type, header->filter.ext_type);
	printk("  Flags:          0x%04X\n", header->filter.flags_bits);
	printk("    data_format:  %u\n", header->filter.flags.data_format);
	printk("    encoding:     %u\n", header->filter.flags.encoding);
	printk("    encrypr_alg:  %u\n", header->filter.flags.encrypt_alg);
	printk("    _rsvd1:       %u\n", header->filter.flags._rsvd);
	printk("\n");
	printk("SrcLen:           0x%08X\n", header->srclen_bits);
	printk("  len:            0x%04X (%u)\n", header->srclen.len, header->srclen.len);
	printk("  is_partial:     %u\n", header->srclen.is_partial);
	printk("  timestamp:      %u\n", header->srclen.timestamp);
	printk("  _rsvd:          %u\n", header->srclen._rsvd);
	printk("  sourceid:       %u\n", header->srclen.sourceid);
	printk("\n");
}

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

static const struct sdp_node g_test_proc_node = {
	/* Filter chain:
	 * This will match if DS filter field = (0x26 OR 0X1026) AND bit 17 is set.
	 */
	.filters = (struct sdp_filter[]){
		{
			.exact_match = 0x00000026,
		},
		{
			.comb_op = SDP_FILTER_COMB_OP_OR,
			.exact_match = 0x00001026,
		},
		{
			.comb_op = SDP_FILTER_COMB_OP_AND,
			.bits = {
				.mask = 0xFFFDFFFF,
				.set = (1 << 17)
			}
		}
	},
	.filter_count = 3,

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

static const struct sdp_ds_header g_test_ds_header = {
	/* Filter word. */
	.filter.data_type = SDP_DS_TYPE_NUMERIC,
	.filter.ext_type = SDP_DS_EXT_TYPE_NUM_IEEE754_F32,
	.filter.flags = {
		.data_format = SDP_DS_FORMAT_TLV,
		.encoding = SDP_DS_ENCODING_NONE,
		.encrypt_alg = SDP_DS_ENCRYPT_ALG_NONE,
	},

	/* Source/Len word. */
	.srclen.len = 0x1FF,
	.srclen.is_partial = 0,
	.srclen.timestamp = 0,
	.srclen.sourceid = 10
};

int main()
{
	uint32_t rc = 0;
	uint8_t handle = 0;

	/* Parse the data sample. */
	print_header(&g_test_ds_header);

	/* Register the processor node. */
	rc = sdp_pm_register(&g_test_proc_node, &handle);

	return 0;
}