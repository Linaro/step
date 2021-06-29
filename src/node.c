/*
 * Copyright (c) 2021 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <sdp/filter.h>
#include <sdp/node.h>

void sdp_node_print(struct sdp_node *node)
{
	/* Filter chain*/
    sdp_filt_print(node->filters);

	/* Callback handlers */
	printk("Handlers:\n");
	printk("  evaluate: %s\n",
	       node->callbacks.evaluate_handler == NULL ? "no" : "yes");
	printk("  matched: %s\n",
	       node->callbacks.matched_handler == NULL ? "no" : "yes");
	printk("  start: %s\n",
	       node->callbacks.start_handler == NULL ? "no" : "yes");
	printk("  run: %s\n",
	       node->callbacks.run_handler == NULL ? "no" : "yes");
	printk("  stop: %s\n",
	       node->callbacks.stop_handler == NULL ? "no" : "yes");
	printk("  error: %s\n",
	       node->callbacks.error_handler == NULL ? "no" : "yes");
}
