/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <sdp/proc_mgr.h>
#include <sdp/filter.h>

static uint32_t g_sdp_pm_handle_counter = 0;

/**
 * @brief Prints full details of the supplied processor node using printk.
 *
 * @param node The node to display.
 */
static void sdp_pm_print_node_full(struct sdp_node *node)
{
	/* Filtering */
	printk("  Filters: %d\n", node->filters.count);
	for (uint8_t i = 0; i < node->filters.count; i++) {
		printk("    #%d: ", i);

		/* Combinatory operand */
		switch (node->filters.chain[i].comb_op) {
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
		switch (node->filters.chain[i].op) {
		case SDP_FILTER_OP_NOT:
			printk("NOT ");
			break;
		default:
			break;
		}

		/* Exact match or bit-based */
		if (node->filters.chain[i].exact_match) {
			printk("exact match: 0x%08X\n", node->filters.chain[i].exact_match);
			printk("exact match: 0x%08X", node->filters.chain[i].exact_match);
			if (node->filters.chain[i].exact_match_mask) {
				printk(" (mask 0x%08X)\n",
				       node->filters.chain[i].exact_match_mask);
			} else {
				printk("\n");
			}
		} else {
			printk("bit match: mask 0x%08X set 0x%08X cleared 0x%08X\n",
			       node->filters.chain[i].bit_match.mask,
			       node->filters.chain[i].bit_match.set,
			       node->filters.chain[i].bit_match.cleared);
		}
	}

	/* Callback handlers */
	printk("  Handlers:\n");
	printk("    evaluate: %s\n",
	       node->callbacks.evaluate_handler == NULL ? "no" : "yes");
	printk("    matched: %s\n",
	       node->callbacks.matched_handler == NULL ? "no" : "yes");
	printk("    start: %s\n",
	       node->callbacks.start_handler == NULL ? "no" : "yes");
	printk("    run: %s\n",
	       node->callbacks.run_handler == NULL ? "no" : "yes");
	printk("    stop: %s\n",
	       node->callbacks.stop_handler == NULL ? "no" : "yes");
	printk("    error: %s\n",
	       node->callbacks.error_handler == NULL ? "no" : "yes");
}

int sdp_pm_process_ds(struct sdp_datasample *sample)
{
	int rc = 0;

	/* ToDo: Check what processor nodes can handle this ds. */

	return rc;
}

int sdp_pm_poll(void)
{
	int rc = 0;

	/* ToDo: Check sample pool for samples to process. */

	return rc;
}

int sdp_pm_register(struct sdp_node *node, uint8_t *handle)
{
	*handle = ++g_sdp_pm_handle_counter;

	printk("Registering processor node %d:\n", *handle);
	sdp_pm_print_node_full(node);

	return 0;
}

int sdp_pm_list(void)
{
	return 0;
}