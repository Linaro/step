/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <sdp/filter.h>
#include <sdp/datasample.h>

#include <sys/printk.h>

int sdp_filt_evaluate(struct sdp_filter_chain *fc,
		      struct sdp_datasample *sample, int *match)
{
	int rc = 0;
	int curr_match, prev_match;

	*match = 0;

	if ((fc == NULL) || (sample == NULL) || (fc->count == 0)) {
		rc = -EINVAL;
		goto err;
	}

	printk("Sample value: 0x%08X\n", sample->header.filter_bits);

	/* Iterate through filter chain. */
	curr_match = prev_match = 0;
	for (uint32_t i = 0; i < fc->count; i++)
	{
		printk("Evaluating filter #%d - ", i);

		/* Combinatory operand */
		switch (fc->chain[i].comb_op) {
		case SDP_FILTER_COMB_OP_AND:
			printk("AND ");
			break;
		case SDP_FILTER_COMB_OP_OR:
			printk("OR ");
			break;
		case SDP_FILTER_COMB_OP_XOR:
			printk("XOR ");
			break;
		case SDP_FILTER_COMB_OP_NONE:
		default:
			break;
		}

		/* Current operand */
		switch (fc->chain[i].op) {
		case SDP_FILTER_OP_NOT:
			printk("NOT ");
			break;
		default:
			break;
		}

		/* Exact match or bit-based */
		if (fc->chain[i].exact_match) {
			printk("exact match: 0x%08X", fc->chain[i].exact_match);
			if (fc->chain[i].exact_match_mask) {
				printk(" (mask 0x%08X)\n", fc->chain[i].exact_match_mask);
			} else {
				printk("\n");
			}
		} else {
			printk("bit match: mask 0x%08X set 0x%08X cleared 0x%08X\n",
			       fc->chain[i].bit_match.mask,
			       fc->chain[i].bit_match.set,
			       fc->chain[i].bit_match.cleared);
		}
	}

	/* For now, always pass unless we encounter an error. */
	*match = 1;

err:
	return rc;
}
