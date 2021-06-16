/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include "proc_mgr.h"
#include "filter.h"

static uint32_t g_sdp_pm_handle_counter = 0;

static void sdp_pm_print(struct sdp_node *node)
{
	/* Filtering */
	printk("  Filters: %d\n", node->filter_count);
	for (uint8_t i = 0; i < node->filter_count; i++) {
		printk("    #%d: ", i);
		/* Combinatory operand */
		switch (node->filters[i].comb_op) {
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
		switch (node->filters[i].op) {
		case SDP_FILTER_OP_NOT:
			printk("NOT ");
			break;
		default:
			break;
		}
		/* Exact match or bit-based */
		if (node->filters[i].exact_match) {
			printk("exact match: 0x%08X\n", node->filters[i].exact_match);
		} else {
			printk("bit match: mask 0x%08X set 0x%08X cleared 0x%08X\n",
			       node->filters[i].bit_match.mask,
			       node->filters[i].bit_match.set,
			       node->filters[i].bit_match.cleared);
		}
	}

	/* Callback handlers */
	printk("  Handlers:\n");
	printk("    evaluate: %s\n", node->evaluate_handler == NULL ? "no" : "yes");
	printk("    matched: %s\n", node->matched_handler == NULL ? "no" : "yes");
	printk("    start: %s\n", node->start_handler == NULL ? "no" : "yes");
	printk("    run: %s\n", node->run_handler == NULL ? "no" : "yes");
	printk("    stop: %s\n", node->stop_handler == NULL ? "no" : "yes");
	printk("    error: %s\n", node->error_handler == NULL ? "no" : "yes");
}

int sdp_pm_register(struct sdp_node *node, uint8_t *handle)
{
	*handle = g_sdp_pm_handle_counter++;

	printk("Registering processor node %d:\n", *handle);
    sdp_pm_print(node);

    return 0;
}